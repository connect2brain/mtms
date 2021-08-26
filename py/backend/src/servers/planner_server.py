#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import json
from typing import Any, Callable, Dict, List, Literal, Optional, Tuple, TypedDict

from socketio import AsyncServer

from mtms.common.util import clamp, drop_unique
from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener

from mtms.common.constants import KAFKA_COMMAND_SET_STIMULATION_PARAMETERS, KAFKA_COMMAND_SET_COIL_AT_TARGET
from mtms.common.types import Intensity, Iti, StimulationParameters

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Types related to planner.

class SetIntensityForPointData(TypedDict):
    name: str
    value: Intensity

class SetItiForPointData(TypedDict):
    name: str
    value: Iti

# XXX: Position and Direction probably shouldn't be optional.
#
Position = Optional[Tuple[float, float, float]]
Direction = Optional[Tuple[float, float, float]]

Color = Tuple[int, int, int]

class AddPointData(TypedDict):
    position: Position

class PointSelectedData(TypedDict):
    name: str

FiducialName = Literal['LE', 'RE', 'NA']
FiducialType = Literal['image', 'tracker']

class Fiducial(TypedDict):
    name: FiducialName
    type: FiducialType

class SetFiducialData(TypedDict):
    fiducial: Fiducial

class Point(TypedDict):
    visible: bool
    name: str
    type: str
    comment: str
    selected: bool
    target: bool
    position: Position
    intensity: Intensity
    iti: Iti

class FiducialSet(TypedDict):
    fiducial: Fiducial
    position: Position

class NeuroNavigationMessage(TypedDict):
    topic: str
    data: Any

class PlannerServer:
    """A backend server for the planner.

    """

    # Socket.IO events sent by the planner.
    #
    # TODO: Whether the event is sent or received by the planner should be reflected by the event name.
    #
    # XXX: Should there be another server for calibration-related functions? (e.g., 'calibration.fiducial_set' below)
    #
    _SOCKETIO_UPDATE_NAVIGATING: str = 'planner.navigating'
    _SOCKETIO_UPDATE_POINTS: str = 'planner.points'
    _SOCKETIO_UPDATE_POSITION: str = 'planner.position'
    _SOCKETIO_UPDATE_COIL_AT_TARGET: str = 'planner.coil_at_target'
    _SOCKETIO_STATE_SENT: str = 'planner.state_sent'

    _SOCKETIO_FIDUCIAL_SET: str = 'calibration.fiducial_set'

    _KAFKA_COMMAND_ADD_POINT: str = 'point.add'
    _KAFKA_COMMAND_REMOVE_POINT: str = 'point.remove'
    _KAFKA_COMMAND_SET_POINT_INTENSITY: str = 'point.set_intensity'
    _KAFKA_COMMAND_SET_POINT_ITI: str = 'point.set_iti'

    _KAFKA_COMMAND_SET_FIDUCIAL: str = 'calibration.set_fiducial'

    # Stimulation parameter related constants.
    #
    _INTENSITY: Dict[str, int] = {
        'initial': 100,
        'min': 1,
        'max': 1000,
    }
    _ITI: Dict[str, int] = {
        'initial': 100,
        'min': 10,
        'max': 1000,
    }

    # The colors have been picked from mTMS software prototype created in Adobe XD.
    #
    # XXX: The colors match those that are defined in _colors.scss in front-end. It would be
    #      better if they were defined in a single place.
    #
    _COLOR_TARGET: Color = (43, 197, 255)  # hex: #2BC5FF, $target-color
    _COLOR_NON_TARGET: Color = (230, 98, 48)  # hex: #E66230, $non-target-color
    _COLOR_SELECTED: Color = (112, 112, 112)  # hex: #707070, $darker-gray

    def __init__(self, kafka: Kafka, socketio: AsyncServer) -> None:
        """Initialize the planner server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            An AsyncServer object to which the event listeners are added.
        """
        self._socketio: AsyncServer = socketio
        self._kafka: Kafka = kafka

        self._position: Position = None
        self._neuronavigation_project_open: bool = False
        self._coil_at_target: bool = False
        self._navigating: bool = False

        self._fiducials_set: Dict[FiducialType, Dict[FiducialName, bool]] = {
            "image": {},
            "tracker": {},
        }

        self._added_points_total: int = 0

        # Set up Socket.IO event handlers

        self._SOCKETIO_EVENT_HANDLERS: Dict[str, Callable] = {
            # Neuronavigation sends a message.
            'from_neuronavigation': self._handle_neuronavigation_message,

            # Add a new point in the front-end.
            #
            # XXX: Unify the naming, cf. 'planner.points.selected' below.
            #
            'point.add': self._handle_add_point,

            # Remove a point in the front-end
            'point.remove': self._handle_remove_point,

            # Front-end requesting state
            'planner.request_state': self._handle_state_request,

            # A fiducial is set in the front-end
            'calibration.set_fiducial': self._handle_set_fiducial,

            # Toggle selecting a point in the front-end
            'planner.point.toggle_select': self._handle_point_toggle_selected,

            # Toggle a point as target in the front-end
            'planner.point.toggle_target': self._handle_point_toggle_target,

            # Set intensity of a point in the front-end
            'planner.point.set_intensity': self._handle_point_set_intensity,

            # Set ITI of a point in the front-end
            'planner.point.set_iti': self._handle_point_set_iti,

            # Toggle navigation in the front-end
            'planner.toggle_navigating': self._handle_toggle_navigating,
        }

        for event, handler in self._SOCKETIO_EVENT_HANDLERS.items():
            socketio.on(
                event=event,
                handler=handler,
            )

        self._points: List[Point] = []

        self._setup_background_tasks()

    def _setup_background_tasks(self):
        """Setup the background tasks, namely, a task for listening for a Kafka command
        to add a new point.

        """
        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=self._KAFKA_COMMAND_ADD_POINT,
                callback=self._point_added,
            ),
            KafkaListener(
                kafka=self._kafka,
                topic=self._KAFKA_COMMAND_REMOVE_POINT,
                callback=self._point_removed,
            ),
            KafkaListener(
                kafka=self._kafka,
                topic=self._KAFKA_COMMAND_SET_FIDUCIAL,
                callback=self._fiducial_set,
            ),
        ]

    async def _handle_neuronavigation_message(self, client_id: str, msg: NeuroNavigationMessage) -> None:
        """A handler for messages from neuronavigation.

        Broadcasts the message received from neuronavigation.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        msg
            The message received from neuronavigation, see type NeuroNavigationMessage for the structure.
        """
        topic: str = msg['topic']
        data: Any = msg['data']
        logging.info("Received a message from neuronavigation in topic '{}'".format(topic))

        # Triggered when the cross is moved around in neuronavigation. React by updating the new cross
        # position to the frontend.
        if topic == "Set cross focal point":
            self._position = data['position'][0:3]
            await self._update_position()

        # Triggered when a project is opened in neuronavigation. React by sending the current points to
        # neuronavigation so that they can be shown in the project.
        elif topic == "Enable state project":
            self._neuronavigation_project_open = data['state']
            if self._neuronavigation_project_open:
                await self._update_neuronavigation()

        # Triggered when the neuronavigation evaluates if the coil is at the target: accordingly, the state
        # is either True or False.
        elif topic == "Coil at target":
            self._coil_at_target = data['state']
            await self._update_coil_at_target()

    def _handle_add_point(self, client_id: str, data: AddPointData) -> None:
        """When a command is received from the front-end to add a new point, create the
        initial values for the point and pass the message to create a point to Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end with the command. Should consist of
            'position' key, with a value that consists of a list of three floats.

            See type AddPointData for the specification.

            An example:

            {
                'position': [1.0, 2.0, 3.0],
            }
        """
        logging.info("Received a command from the front-end to add a new point")

        self._added_points_total += 1

        # XXX: Use constant direction for all points. This needs to be thought through.
        #
        direction: Direction = [0.0, 0.0, 0.0]

        value: Point = {
            'visible': False,
            'name': "Target-{}".format(self._added_points_total),
            'type': "Target",
            'comment': "",
            'selected': False,
            'target': False,
            'position': data['position'],
            'direction': direction,

            # Stimulation-related parameters
            'intensity': self._INTENSITY['initial'],
            'iti': self._ITI['initial'],
        }

        self._kafka.produce(
            topic=self._KAFKA_COMMAND_ADD_POINT,
            value=bytes(json.dumps(value), encoding='utf8')
        )

    def _handle_remove_point(self, client_id: str, data: PointSelectedData) -> None:
        """When a command is received from the front-end to remove a point, pass the
        message to remove a point to Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end with the command.

            See type PointSelectedData for the specification.

            An example:

            {
                'name': "Target-1",
            }
        """
        logging.info("Received a command from the front-end to remove a point")

        self._kafka.produce(
            topic=self._KAFKA_COMMAND_REMOVE_POINT,
            value=bytes(json.dumps(data), encoding='utf8')
        )

    async def _handle_point_toggle_selected(self, client_id: str, data: PointSelectedData) -> None:
        """When a command is received from the front-end to select points, pass the
        message to neuronavigation.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end.

            See type PointSelectedData for the specification.

            An example:

            {
                'name': "Target-1",
            }
        """
        logging.info("Received a command from the front-end to toggle selecting a point")

        name: str = data['name']

        point: Point
        for point in self._points:
            if point['name'] == name:
                point['selected'] = not point['selected']

        # Update neuronavigation
        await self._update_neuronavigation()

        # Update frontend
        await self._update_points()

    async def _handle_point_toggle_target(self, client_id: str, data: PointSelectedData) -> None:
        """When a command is received from the front-end to toggle a target point, pass on
        the message to neuronavigation.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end.

            See type PointSelectedData for the specification.

            An example:

            {
                'name': "Target-1",
            }
        """
        logging.info("Received a command from the front-end to toggle a point as target")

        name: str = data['name']

        # TODO: Ensure that only a single point is set as target.
        #
        point: Point
        for point in self._points:
            if point['name'] == name:
                point['target'] = not point['target']
            else:
                point['target'] = False

        # Update neuronavigation
        await self._update_neuronavigation()

        # Update frontend
        await self._update_points()

        # Update stimulation parameters due to a new target
        self._update_current_stimulation_parameters_to_kafka()

    # TODO: Ideally, we'd have another class that takes care of transforming
    #       the planner state to current stimulation parameters and keeping
    #       Kafka up to date. That would also facilitate testing.
    #
    def _update_current_stimulation_parameters_to_kafka(self) -> None:
        """Update current stimulation parameters (intensity, ITI) to Kafka.

        """
        # XXX: Ensure that only a single target point is found. See similar concerns
        #      elsewhere in this class.
        #
        target_point: Optional[Point] = None
        for point in self._points:
            if point['target']:
                target_point = point

        if target_point is None:
            return

        intensity: Intensity = target_point['intensity']
        iti: Iti = target_point['iti']

        stimulation_parameters: StimulationParameters = {
            'intensity': intensity,
            'iti': iti,
        }

        self._kafka.produce(
            topic=KAFKA_COMMAND_SET_STIMULATION_PARAMETERS,
            value=bytes(json.dumps(stimulation_parameters), encoding='utf8'),
        )

    async def _handle_point_set_intensity(self, client_id: str, data: SetIntensityForPointData) -> None:
        """When a command is received from the front-end to set intensity, pass on
        the message to Kafka and modify the backend state.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end.

            See type SetIntensityData for the specification.

            An example:

            {
                'name': "Target-1",
                'value': 100,
            }
        """
        logging.info("Received a command from the front-end to set intensity for a point")

        name: str = data['name']
        value: int = data['value']

        clamped: int = clamp(value, self._INTENSITY["min"], self._INTENSITY["max"])

        # TODO: Ensure that only a single point changes intensity, see a similar concern
        #       in _handle_point_toggle_target.
        #
        point: Point
        for point in self._points:
            if point['name'] == name:
                point['intensity'] = clamped

        data_to_kafka = {
            'name': name,
            'value': clamped,
        }

        self._kafka.produce(
            topic=self._KAFKA_COMMAND_SET_POINT_INTENSITY,
            value=bytes(json.dumps(data_to_kafka), encoding='utf8')
        )

        # Update frontend
        await self._update_points()

        # Update stimulation parameters due to a new intensity for a potential target.
        #
        # XXX: This could be cleaner: it would be better to only update stimulation
        #      parameters when a new intensity/ITI for the actual target is set.
        #
        self._update_current_stimulation_parameters_to_kafka()

    # TODO: Almost identical to _handle_point_set_intensity. Consider extracting out the
    #       common parts.
    #
    async def _handle_point_set_iti(self, client_id: str, data: SetItiForPointData) -> None:
        """When a command is received from the front-end to set ITI, pass on
        the message to Kafka and modify the backend state.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end.

            See type SetItiData for the specification.

            An example:

            {
                'name': "Target-1",
                'value': 100,
            }
        """
        logging.info("Received a command from the front-end to set ITI for a point")

        name: str = data['name']
        value: int = data['value']

        clamped: int = clamp(value, self._ITI["min"], self._ITI["max"])

        # TODO: Ensure that only a single point changes ITI, see a similar concern
        #       in _handle_point_toggle_target.
        #
        point: Point
        for point in self._points:
            if point['name'] == name:
                point['iti'] = clamped

        data_to_kafka = {
            'name': name,
            'value': clamped,
        }

        self._kafka.produce(
            topic=self._KAFKA_COMMAND_SET_POINT_ITI,
            value=bytes(json.dumps(data_to_kafka), encoding='utf8')
        )

        # Update frontend
        await self._update_points()

        # Update stimulation parameters due to a new ITI for a potential target.
        #
        # XXX: This could be cleaner: it would be better to only update stimulation
        #      parameters when a new intensity/ITI for the actual target is set.
        #
        self._update_current_stimulation_parameters_to_kafka()

    async def _handle_toggle_navigating(self, client_id: str) -> None:
        """When a command is received from the front-end to toggle navigating, pass on
        the message to neuronavigation.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        """
        logging.info("Received a command from the front-end to toggle navigation")

        # Update backend
        self._navigating = not self._navigating

        # Update neuronavigation
        #
        topic = "Start navigation" if self._navigating else "Stop navigation"
        await self._send_to_neuronavigation(
            topic=topic,
        )

        # Update frontend
        await self._update_navigating()

    def _handle_set_fiducial(self, client_id: str, data: SetFiducialData) -> None:
        """When a command is received from the front-end to set a fiducial, pass the
        message to Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end with the fiducial data.

            See type SetFiducialData for the specification.

            An example:

            {
                'fiducial': {
                    'name': "LE",
                    'type': "image",
                }
            }
        """
        logging.info("Received a command from the front-end to set a fiducial")

        fiducial = data['fiducial']

        value: FiducialSet = {
            'fiducial': fiducial,

            # XXX: Note that when adding a point, the position makes a round-trip to the front-end,
            #      whereas here the backend sets the position directly. The logic should be unified.
            #
            # XXX: Note also that this is unused if a tracker fiducial is being set. Should it be removed in that case?
            #      Also consider renaming to 'image_position' to distinguish it from tracker position.
            #
            'position': self._position,
        }

        self._kafka.produce(
            topic=self._KAFKA_COMMAND_SET_FIDUCIAL,
            value=bytes(json.dumps(value), encoding='utf8')
        )

    async def _handle_state_request(self, client_id: str, _) -> None:
        """When the state is requested by the client, send all needed data, e.g., the points
        and the position.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        """
        logging.info("Planner state requested by the client with the id {}".format(client_id))

        await self._update_points(
            client_id=client_id,
        )
        await self._update_position(
            client_id=client_id,
        )
        await self._update_coil_at_target(
            client_id=client_id,
        )
        await self._update_navigating(
            client_id=client_id,
        )
        await self._socketio.emit(
            event=self._SOCKETIO_STATE_SENT,
            client_id=client_id,
        )

    async def _update_points(self, client_id: str = None) -> None:
        """Update planner points to the frontend.

        Parameters
        ----------
        client_id
            The client id. If provided, the points are sent only to the client with that id.
            If not provided (the default), the points are broadcast to all clients.
        """
        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_POINTS,
            data=self._points,
            client_id=client_id,
        )

    async def _update_position(self, client_id: str = None) -> None:
        """Update the position, if defined, to the frontend.

        Parameters
        ----------
        client_id
            The client id. If provided, the position is sent only to the client with that id.
            If not provided (the default), the position is broadcast to all clients.
        """
        if self._position is not None:
            await self._socketio.emit(
                event=self._SOCKETIO_UPDATE_POSITION,
                data=self._position,
                client_id=client_id,
            )

    async def _update_navigating(self, client_id: str = None) -> None:
        """Update the navigating state to the frontend.

        Parameters
        ----------
        client_id
            The client id. If provided, the position is sent only to the client with that id.
            If not provided (the default), the position is broadcast to all clients.
        """
        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_NAVIGATING,
            data=self._navigating,
            client_id=client_id,
        )

    async def _update_coil_at_target(self, client_id: str = None) -> None:
        """Update the indicator for coil being at the target, both to Kafka and frontend.

        Parameters
        ----------
        client_id
            The client id. If provided, the position is sent only to the client with that id.
            If not provided (the default), the position is broadcast to all clients.
        """
        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_COIL_AT_TARGET,
            data=self._coil_at_target,
            client_id=client_id,
        )

        self._kafka.produce(
            topic=KAFKA_COMMAND_SET_COIL_AT_TARGET,

            # XXX: Unify the encoding of parameters of boolean (or other) types, preferably
            #      using a wrapper class that does the encoding instead of doing it ad hoc,
            #      even though the encoding here is just True -> b"True" and False -> b"False".
            #
            value=bytes(str(self._coil_at_target), encoding='utf8'),
        )

    async def _send_to_neuronavigation(self, topic: str, data: Any = None) -> None:
        """Given a topic and data of any type, send a message to neuronavigation in that topic
        and passing on the given data.

        Parameters
        ----------
        topic
            The topic for the internal communication that the neuronavigation software does. E.g.,
            "Add marker".
        data
            Any data that will be sent in the given topic.

        """
        await self._socketio.emit(
            event="to_neuronavigation",
            data={
                "topic": topic,
                "data": data,
            }
        )

    async def _update_neuronavigation(self) -> None:
        """Send all points to neuronavigation to display them.

        """
        markers = []

        for id, point in enumerate(self._points):
            position: Position = point['position']
            direction: Direction = point['direction']
            selected: bool = point['selected']
            target: bool = point['target']

            color: Color
            if target:
                color = self._COLOR_TARGET

            elif selected:
                color = self._COLOR_SELECTED

            else:
                color = self._COLOR_NON_TARGET

            marker_data = {
                'ball_id': id,
                'position': position,
                'direction': direction,
                'target': target,
                'size': 3,
                'colour': [c / 255.0 for c in color],
            }
            markers.append(marker_data)

        await self._send_to_neuronavigation(
            topic="Set markers",
            data={
                "markers": markers,
            }
        )

    async def _send_fiducial_to_neuronavigation(self,
            fiducial_type: FiducialType,
            fiducial_name: FiducialName,
            position: Position) -> None:
        """Send fiducial to neuronavigation.

        """
        if fiducial_type == "image":

            await self._send_to_neuronavigation(
                topic="Set image fiducial",
                data={
                    "fiducial_name": fiducial_name,
                    "coord": position,
                }
            )

        elif fiducial_type == "tracker":

            # XXX: 'position' is unused if fiducial type is tracker.
            await self._send_to_neuronavigation(
                topic="Set tracker fiducial",
                data={
                    "fiducial_name": fiducial_name,
                }
            )

        else:
            assert False, "Unknown fiducial type: {}".format(fiducial_type)

    async def _point_added(self, topic: str, data: str) -> None:
        """Handle the command received from Kafka to add a new point, specifically, pass the
        command on to the front-end and neuronavigation.

        Parameters
        ----------
        topic
            The name of the topic in which the command is sent.
        data
            A json dict consisting of the attributes of the new point. The dict should
            consist of the keys 'visible', 'name', 'type', 'comment', and 'position'.

            An example:

            {
                'visible': False,
                'name': "Target-1",
                'type': "Target",
                'comment': "This is a comment.",
                'position': [1.0, 2.0, 3.0],
            }
        """
        # TODO: The format and the content of the received message needs to be checked.
        #       A natural place for those checks would be inside Kafka listener, so this
        #       function can then assume a valid message.
        data: Point = json.loads(data)

        # Update backend
        self._points.append(data)

        # Update frontend
        await self._update_points()

        # Update neuronavigation
        await self._update_neuronavigation()

    async def _point_removed(self, topic: str, data: str) -> None:
        """Handle the command received from Kafka to remove a point.

        Parameters
        ----------
        topic
            The name of the topic in which the command is sent.
        data
            A json dict consisting of the attributes of the new point. The dict should
            consist of the key 'name'.

            An example:

            {
                'name': "Target-1",
            }
        """
        data: PointSelectedData = json.loads(data)

        # Update backend
        index: int = drop_unique(self._points, lambda point: point['name'] == data['name'])

        # Update frontend
        await self._update_points()

        # Update neuronavigation
        await self._update_neuronavigation()

    async def _fiducial_set(self, topic: str, data: str) -> None:
        """Handle the command received from Kafka to set a fiducial.

        Parameters
        ----------
        topic
            The name of the topic in which the command is sent.
        data
            A json dict consisting of the attributes of the fiducial. The dict should
            consist of the keys 'fiducial_name' and 'position'.

            An example:

            {
                'fiducial': {
                    'name': "LE",
                    'type': "image",
                },
                'position': [1.0, 2.0, 3.0],
            }
        """
        data: FiducialSet = json.loads(data)

        fiducial = data['fiducial']
        position = data['position']

        fiducial_type = fiducial['type']
        fiducial_name = fiducial['name']

        # Update backend
        self._fiducials_set[fiducial_type][fiducial_name] = True

        # Update neuronavigation
        await self._send_fiducial_to_neuronavigation(
            fiducial_type=fiducial_type,
            fiducial_name=fiducial_name,
            position=position,
        )

        # Broadcast a confirmation that the fiducial has been set.
        await self._socketio.emit(
            event=self._SOCKETIO_FIDUCIAL_SET,
            data=fiducial,
        )
