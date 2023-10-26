import time
from threading import Event

import numpy as np

from rclpy.executors import MultiThreadedExecutor

from event_interfaces.msg import ExecutionCondition, Charge, Discharge, ChargeFeedback, DischargeFeedback, EventInfo

from mtms_device_interfaces.msg import SystemState, SessionState, DeviceState
from mtms_device_interfaces.action import SetVoltages
from utility_interfaces.srv import GetNextId

import rclpy
from rclpy.action import ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class VoltageSetterNode(Node):

    # TODO: Channel count hardcoded for now.
    NUM_OF_CHANNELS = 5

    # TODO: Hardcoded for now.
    MAXIMUM_VOLTAGE = 1500

    def __init__(self):
        super().__init__('voltage_setter_node')

        self.logger = self.get_logger()

        self.callback_group = ReentrantCallbackGroup()

        # Create action server for setting voltages.

        self.action_server = ActionServer(
            self,
            SetVoltages,
            '/mtms_device/set_voltages',
            self.handle_action_set_voltages,
            callback_group=self.callback_group,
        )

        # Create service client for getting next ID.
        self.get_next_id_client = self.create_client(GetNextId, '/utility/get_next_id', callback_group=self.callback_group)
        while not self.get_next_id_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /utility/get_next_id not available, waiting...')

        # Have a queue of only one message so that only the latest system state is received.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1, callback_group=self.callback_group)
        self.system_state = None

        # Create publishers for charging and discharging.
        self.send_charge_publisher = self.create_publisher(Charge, '/event/send/charge', 10)
        self.send_discharge_publisher = self.create_publisher(Discharge, '/event/send/discharge', 10)

        self.charge_feedback_subscriber = self.create_subscription(ChargeFeedback, '/event/charge_feedback', self.update_event_feedback, 10)
        self.discharge_feedback_subscriber = self.create_subscription(DischargeFeedback, '/event/discharge_feedback', self.update_event_feedback, 10)

        self.event_feedback = {}

    ## ROS callbacks and callers

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def get_device_state(self):
        return self.system_state.device_state.value

    def is_device_started(self):
        return self.get_device_state() == DeviceState.OPERATIONAL

    def get_session_state(self):
        return self.system_state.session_state.value

    def is_session_started(self):
        return self.get_session_state() == SessionState.STARTED

    def get_current_time(self):
        return self.system_state.time

    # Feedback

    def update_event_feedback(self, feedback):
        id = feedback.id
        error = feedback.error

        # TODO: Improve feedback logging, preferably by implementing events as ROS actions.
        self.logger.info('Event {} finished with error code: {}'.format(id, error.value))

        self.event_feedback[id] = error

    def get_event_feedback(self, id):
        if id not in self.event_feedback:
            return None

        return self.event_feedback[id]

    # Utilities

    def get_voltage(self, channel):
        return self.system_state.channel_states[channel].voltage

    # Set voltages action

    def handle_action_set_voltages(self, goal_handle):
        request = goal_handle.request

        voltages = request.voltages

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        success = self.perform_set_voltages(
            goal_id=goal_id,
            voltages=voltages,
        )

        # Create and return a Result object.
        result = SetVoltages.Result()

        goal_handle.succeed()

        result.success = success
        self.logger.info('{}: Done.'.format(goal_id))

        return result

    ## Service calls

    def async_service_call(self, client, request):
        call_service_event = Event()
        response_value = [None]

        def service_call_callback(future):
            nonlocal response_value
            response_value[0] = future.result()
            call_service_event.set()

        service_call_future = client.call_async(request)
        service_call_future.add_done_callback(service_call_callback)

        # Wait for the service call to complete
        call_service_event.wait()

        response = response_value[0]
        return response

    # Utility services

    def get_next_id(self):
        request = GetNextId.Request()

        response = self.async_service_call(self.get_next_id_client, request)
        assert response.success, "Getting next ID failed."

        return response.id

    # Charging and discharging

    def immediately_charge(self, channel, target_voltage):
        message = Charge()

        id = self.get_next_id()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = ExecutionCondition.IMMEDIATE
        event_info.execution_time = 0.0
        event_info.delay = 0.0

        message.event_info = event_info
        message.channel = channel
        message.target_voltage = target_voltage

        self.send_charge_publisher.publish(message)

        self.event_feedback[id] = None

        return id

    def immediately_discharge(self, channel, target_voltage):
        message = Discharge()

        id = self.get_next_id()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = ExecutionCondition.IMMEDIATE
        event_info.execution_time = 0.0
        event_info.delay = 0.0

        message.event_info = event_info
        message.channel = channel
        message.target_voltage = target_voltage

        self.send_discharge_publisher.publish(message)

        self.event_feedback[id] = None

        return id

    def set_voltages(self, goal_id, voltages):
        ids = [None] * self.NUM_OF_CHANNELS
        for channel in range(self.NUM_OF_CHANNELS):
            target_voltage = voltages[channel]

            current_voltage = self.get_voltage(channel)

            is_charging = current_voltage < target_voltage
            charge_or_discharge_function = self.immediately_charge if is_charging else self.immediately_discharge

            self.logger.info('{}: Channel {}: Current voltage is {} V, {} to {} V.'.format(
                goal_id,
                channel,
                current_voltage,
                "charging" if is_charging else "discharging",
                target_voltage
            ))

            id = charge_or_discharge_function(
                channel=channel,
                target_voltage=target_voltage,
            )
            ids[channel] = id

        return ids

    # General

    def wait_for_events_to_finish(self, ids):
        while True:
            feedbacks = [self.get_event_feedback(id) for id in ids]
            if all([x is not None for x in feedbacks]):
                break

            # XXX: This will cause an extra delay of up to 0.1 seconds. However, the correct
            #   fix would be to implement pulses as ROS actions, which removes the need
            #   for this kind of a check. Aim directly at that, hence settle on this for now.
            time.sleep(0.1)

        return feedbacks

    def check_goal_feasible(self, goal_id):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('{}: mTMS device not started, aborting.'.format(goal_id))
            return False

        # Check that the session is started.
        if not self.is_session_started():
            self.logger.info('{}: Session not started on the mTMS device, aborting.'.format(goal_id))
            return False

        return True

    def perform_set_voltages(self, goal_id, voltages):
        success = self.check_goal_feasible(goal_id)
        if not success:
            return False

        # XXX: Keeping track of the IDs is a bit messy; should use ROS actions instead
        #   to hide the logic.
        ids = self.set_voltages(
            goal_id=goal_id,
            voltages=voltages,
        )
        self.logger.info('{}: Waiting for charging and discharging to finish...'.format(goal_id))

        feedbacks = self.wait_for_events_to_finish(ids)

        # XXX: Should check charge feedbacks separately from discharge feedbacks, as the error codes
        #   could in theory differ. (Currently they do not, in particular the 'no error' condition does not,
        #   hence it can be done like this for now.)
        success = all([feedback.value == 0 for feedback in feedbacks])

        self.logger.info('{}: Done! Setting voltages {}.'.format(
            goal_id,
            'was successful' if success else 'failed'
        ))

        return success

def main(args=None):
    rclpy.init(args=args)

    voltage_setter_node = VoltageSetterNode()

    # Allow several actions to be executed concurrently.
    #
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(voltage_setter_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
