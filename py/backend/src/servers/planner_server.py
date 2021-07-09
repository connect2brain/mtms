#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import json
from functools import partial
from typing import Any, Callable, Dict, List, Literal, Optional, Tuple, TypedDict

from socketio import AsyncServer

from mtms.util.util import drop_unique
from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

Position = Optional[Tuple[float, float, float]]

class AddPointData(TypedDict):
    position: Position

class RemovePointData(TypedDict):
    name: str

class PointsSelectedData(TypedDict):
    names: List[str]

FiducialName = Literal['LE', 'RE', 'NA']
FiducialType = Literal['image', 'tracker']

class Fiducial(TypedDict):
    name: FiducialName
    type: FiducialType

class SetFiducialData(TypedDict):
    fiducial: Fiducial

class PointAddedData(TypedDict):
    visible: bool
    name: str
    type: str
    comment: str
    selected: bool
    position: Position

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
    _SOCKETIO_UPDATE_POINTS: str = 'planner.points'
    _SOCKETIO_UPDATE_POSITION: str = 'planner.position'
    _SOCKETIO_UPDATE_COIL_AT_TARGET: str = 'planner.coil_at_target'
    _SOCKETIO_STATE_SENT: str = 'planner.state_sent'
    _SOCKETIO_FIDUCIAL_SET: str = 'calibration.fiducial_set'

    _KAFKA_COMMAND_ADD_POINT: str = 'point.add'
    _KAFKA_COMMAND_REMOVE_POINT: str = 'point.remove'
    _KAFKA_COMMAND_SET_FIDUCIAL: str = 'calibration.set_fiducial'

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

            # Remove a point in the front-end.
            'point.remove': self._handle_remove_point,

            # Front-end requesting state.
            'planner.request_state': self._handle_state_request,

            # A fiducial is set in the front-end.
            'calibration.set_fiducial': self._handle_set_fiducial,

            # Select points in the front-end.
            'planner.points.selected': self._handle_points_selected,
        }

        for event, handler in self._SOCKETIO_EVENT_HANDLERS.items():
            socketio.on(
                event=event,
                handler=handler,
            )

        self._points: List[PointAddedData] = []

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
        value: PointAddedData = {
            'visible': False,
            'name': "Target-{}".format(self._added_points_total),
            'type': "Target",
            'comment': "",
            'selected': False,
            'position': data['position'],
        }

        self._kafka.produce(
            topic=self._KAFKA_COMMAND_ADD_POINT,
            value=bytes(json.dumps(value), encoding='utf8')
        )

    def _handle_remove_point(self, client_id: str, data: RemovePointData) -> None:
        """When a command is received from the front-end to remove a point, pass the
        message to remove a point to Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end with the command.
            See type RemovePointData for the specification.

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

    async def _handle_points_selected(self, client_id: str, data: PointsSelectedData) -> None:
        """When a command is received from the front-end to select points, pass the
        message to neuronavigation.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end.

            See type PointsSelectedData for the specification.

            An example:

            {
                'names': ["Target-1", "Target-2"],
            }
        """
        logging.info("Received a command from the front-end to select points")

        names = data['names']
        for point in self._points:
            point['selected'] = point['name'] in names

        # Update neuronavigation
        await self._update_neuronavigation()

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
        await self._socketio.emit(
            event=self._SOCKETIO_STATE_SENT,
#            data=self._points,
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

    async def _update_coil_at_target(self, client_id: str = None) -> None:
        """Update the indicator for coil being at the target.

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

    async def _send_to_neuronavigation(self, topic: str, data: Any) -> None:
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
            coord: Position = point['position']
            selected: bool = point['selected']

            marker_data = {
                'ball_id': id,
                'coord': coord,
                'size': 3,

                # XXX: The presentation layer (i.e., neuronavigation) should decide the colours.
                #
                # TODO: Re-think the colour scheme; these are just placeholder colours.
                #
                'colour': (0.0, 1.0, 0.0) if selected else (1.0, 1.0, 0.0),
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
        data: PointAddedData = json.loads(data)

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
        data: RemovePointData = json.loads(data)

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
