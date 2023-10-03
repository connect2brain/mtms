import time
from threading import Event

import numpy as np

from rclpy.executors import MultiThreadedExecutor

from experiment_interfaces.msg import Stimulus, StimulusResult, TriggerConfig
from experiment_interfaces.action import PerformStimulus
from event_interfaces.msg import ExecutionCondition, Pulse, TriggerOut, \
    TriggerOutFeedback, PulseFeedback, EventInfo

from mtms_device_interfaces.msg import SystemState, SessionState, DeviceState
from targeting_interfaces.srv import GetChannelVoltages, GetDefaultWaveform, ReversePolarity
from utility_interfaces.srv import GetNextId

import rclpy
from rclpy.action import ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class StimulusPerformerNode(Node):

    # TODO: Channel count hardcoded for now.
    NUM_OF_CHANNELS = 5

    TRIGGER_DURATION_US = 1000

    def __init__(self):
        super().__init__('stimulus_performer_node')

        self.logger = self.get_logger()

        self.callback_group = ReentrantCallbackGroup()

        # Create action server for performing stimulus.

        self.action_server = ActionServer(
            self,
            PerformStimulus,
            '/stimulus/perform',
            self.perform_stimulus_action_handler,
            callback_group=self.callback_group,
        )

        # Create service client for getting next ID.
        self.get_next_id_client = self.create_client(GetNextId, '/utility/get_next_id', callback_group=self.callback_group)
        while not self.get_next_id_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /utility/get_next_id not available, waiting...')

        # Create service client for targeting.
        self.targeting_client = self.create_client(GetChannelVoltages, '/targeting/get_channel_voltages', callback_group=self.callback_group)
        while not self.targeting_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /targeting/get_channel_voltages not available, waiting...')

        # Create service client for waveforms.
        self.waveform_client = self.create_client(GetDefaultWaveform, '/waveforms/get_default', callback_group=self.callback_group)
        while not self.waveform_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /waveforms/get_default not available, waiting...')

        # Create service client for reversing polarity.
        self.reverse_polarity_client = self.create_client(ReversePolarity, '/waveforms/reverse_polarity', callback_group=self.callback_group)
        while not self.reverse_polarity_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /waveforms/reverse_polarity not available, waiting...')

        # Create subscriber for system state.

        # Have a queue of only one message so that only the latest system state is ever received.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1, callback_group=self.callback_group)
        self.system_state = None

        # Create publishers for charging, discharging, pulse, and trigger out.
        self.send_pulse_publisher = self.create_publisher(Pulse, '/event/send/pulse', 10, callback_group=self.callback_group)
        self.send_trigger_out_publisher = self.create_publisher(TriggerOut, '/event/send/trigger_out', 10, callback_group=self.callback_group)

        self.pulse_feedback_subscriber = self.create_subscription(PulseFeedback, '/event/pulse_feedback', self.update_event_feedback, 10, callback_group=self.callback_group)
        self.trigger_out_feedback_subscriber = self.create_subscription(TriggerOutFeedback, '/event/trigger_out_feedback', self.update_event_feedback, 10, callback_group=self.callback_group)

        self.event_id = 0

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

    # Logging

    def log_stimulus_parameters(self, goal_id, time, stimulus):
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Stimulus parameters:'.format(goal_id))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Time: {:.3f}'.format(goal_id, time))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Target:'.format(goal_id))
        self.logger.info('{}:   - (x, y, angle) = ({}, {}, {})'.format(
            goal_id,
            stimulus.target.displacement_x,
            stimulus.target.displacement_y,
            stimulus.target.rotation_angle,
        ))
        self.logger.info('{}:   - Intensity: {} V/m'.format(goal_id, stimulus.intensity))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Trigger 1:'.format(goal_id))
        self.logger.info('{}:   - Enabled: {}'.format(goal_id, stimulus.triggers[0].enabled))
        self.logger.info('{}:   - Delay: {}'.format(goal_id, stimulus.triggers[0].delay))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Trigger 2:'.format(goal_id))
        self.logger.info('{}:   - Enabled: {}'.format(goal_id, stimulus.triggers[1].enabled))
        self.logger.info('{}:   - Delay: {}'.format(goal_id, stimulus.triggers[1].delay))
        self.logger.info('{}:'.format(goal_id))

    # Performing stimulus

    def perform_stimulus_action_handler(self, goal_handle):
        request = goal_handle.request

        stimulus = request.stimulus
        time = request.time

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        success = self.perform_stimulus(
            goal_id=goal_id,
            time=time,
            stimulus=stimulus,
        )

        # Create and return a Result object.
        result = PerformStimulus.Result()

        goal_handle.succeed()

        result.stimulus_result.success = success
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

    # Targeting services

    def get_channel_voltages(self, target, intensity):
        request = GetChannelVoltages.Request()

        request.target = target
        request.intensity = intensity

        response = self.async_service_call(self.targeting_client, request)
        assert response.success, "Invalid displacement, rotation angle, or intensity."

        return response.voltages, response.reversed_polarities

    def get_default_waveform(self, channel):
        request = GetDefaultWaveform.Request()
        request.channel = channel

        response = self.async_service_call(self.waveform_client, request)

        assert response.success, "Invalid channel."

        return response.waveform

    def reverse_polarity(self, waveform):
        request = ReversePolarity.Request()

        request.waveform = waveform

        response = self.async_service_call(self.reverse_polarity_client, request)
        assert response.success, "Reversing polarity unsuccessful."

        return response.waveform

    # Pulse and trigger out services

    def timed_trigger_out(self, id, time, port):
        message = TriggerOut()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = ExecutionCondition.TIMED
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.port = port
        message.duration_us = self.TRIGGER_DURATION_US

        self.send_trigger_out_publisher.publish(message)
        self.event_feedback[id] = None

    def timed_pulse(self, id, time, channel, waveform):
        message = Pulse()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = ExecutionCondition.TIMED
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.channel = channel
        message.waveform = waveform

        self.send_pulse_publisher.publish(message)
        self.event_feedback[id] = None

    def timed_targeted_pulse(self, time, target, intensity):
        _, reversed_polarities = self.get_channel_voltages(target, intensity)

        ids = [None] * self.NUM_OF_CHANNELS
        for channel in range(self.NUM_OF_CHANNELS):
            id = self.get_next_id()

            reverse_polarity = reversed_polarities[channel]
            waveform = self.get_default_waveform(
                channel=channel,
            )
            # TODO: Double-check that reversing works correctly!
            if reverse_polarity:
                waveform = self.reverse_polarity(
                    waveform=waveform,
                )

            self.timed_pulse(
                id=id,
                time=time,
                channel=channel,
                waveform=waveform,
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

    def check_goal_feasible(self, goal_id, time):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('{}: mTMS device not started, aborting.'.format(goal_id))
            return False

        # Check that the session is started.
        if not self.is_session_started():
            self.logger.info('{}: Session not started on the mTMS device, aborting.'.format(goal_id))
            return False

        # Check that the current time is not past the stimulus time.
        current_time = self.get_current_time()
        if current_time > time:
            self.logger.info('{}: Current time ({:.1f}) is past the stimulus time ({:.1f}), aborting.'.format(
                goal_id,
                current_time,
                time
            ))
            return False

        return True

    def perform_stimulus(self, goal_id, time, stimulus):
        self.log_stimulus_parameters(goal_id, time, stimulus)

        success = self.check_goal_feasible(goal_id, time)
        if not success:
            return False

        # XXX: Keeping track of the IDs is a bit messy; should use ROS actions instead
        #   to hide the logic.
        pulse_ids = self.timed_targeted_pulse(
            time=time,
            target=stimulus.target,
            intensity=stimulus.intensity,
        )

        num_of_trigger_ports = len(stimulus.triggers)
        trigger_ids = []
        for i in range(num_of_trigger_ports):
            if stimulus.triggers[i].enabled:
                id = self.get_next_id()
                delayed_time = time + stimulus.triggers[i].delay
                port = i + 1

                self.timed_trigger_out(
                    id=id,
                    time=delayed_time,
                    port=port,
                )
                trigger_ids += [id]

        all_ids = pulse_ids + trigger_ids

        self.logger.info('{}: Waiting for pulse and trigger out(s) to finish...'.format(goal_id))

        feedbacks = self.wait_for_events_to_finish(all_ids)

        # XXX: Should check pulse feedbacks separately from trigger out feedbacks, as the error codes
        #   may differ. (The 'no error' case does not, hence it can be done like this for now.)
        success = all([feedback.value == 0 for feedback in feedbacks])

        self.logger.info('{}: Done! Stimulus {}.'.format(
            goal_id,
            'was successful' if success else 'failed'
        ))

        return success

def main(args=None):
    rclpy.init(args=args)

    stimulus_performer_node = StimulusPerformerNode()

    # Allow several actions to be executed concurrently.
    #
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(stimulus_performer_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
