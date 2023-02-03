import time
from threading import Event

import numpy as np

from eeg_interfaces.msg import AnalyzeMepErrors, EegInfo, GatherEegError, MepError
from eeg_interfaces.action import AnalyzeMep, GatherEeg
from eeg_interfaces.srv import AnalyzeMepService
from rcl_interfaces.msg import ParameterDescriptor, ParameterType

import rclpy
from rclpy.action import ActionClient, ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup

from rclpy.qos import QoSProfile, DurabilityPolicy, HistoryPolicy, ReliabilityPolicy


class GatheredEegBuffer():
    def __init__(self):
        self.eeg_buffer = None
        self.error = None
        self.event = Event()

    def wait(self):
        self.event.wait()
    
    def set(self):
        self.event.set()


class AnalyzeMepNode(Node):

    ROS_ACTION_GATHER_EEG = ('/eeg/gather', GatherEeg)

    def __init__(self):
        super().__init__('analyze_mep_node')

        self.logger = self.get_logger()
        self.callback_group = ReentrantCallbackGroup()

        # Create subscriber for EegInfo.

        qos_persist_latest = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            reliability=ReliabilityPolicy.RELIABLE,
        )
        self.eeg_info_subscriber = self.create_subscription(
            EegInfo,
            '/eeg/info',
            self.eeg_info_callback,
            qos_persist_latest,
            callback_group=self.callback_group,
        )

        # Create action client for gathering EEG data.

        topic, action_type = self.ROS_ACTION_GATHER_EEG

        client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        self.gather_eeg_client = client

        # Create action server for analyzing MEP.

        self.action_server = ActionServer(
            self,
            AnalyzeMep,
            '/emg/analyze_mep',
            self.analyze_mep_action_handler,
            callback_group=self.callback_group,
        )

        # HACK: MATLAB does not currently (version R2022b) support ROS2 actions. Work around
        #   it by creating a service, which produces the same output as the action.
        #
        self.analyze_mep_service = self.create_service(
            AnalyzeMepService,
            '/emg/analyze_mep_service',
            self.analyze_mep_service_handler,
            callback_group=self.callback_group,
        )

    # ROS callbacks and callers

    def eeg_info_callback(self, eeg_info):
        # Update sampling frequency

        self.sampling_frequency = eeg_info.sampling_frequency
        self.logger.info('Sampling frequency updated to {} Hz.'.format(self.sampling_frequency))

    def call_action__gather_eeg__async(self, goal_id, start_time, end_time, gathered_eeg_buffer):
        topic, action_type = self.ROS_ACTION_GATHER_EEG

        client = self.gather_eeg_client

        # Define goal.
        goal = action_type.Goal()

        goal.time_window.start = start_time
        goal.time_window.end = end_time

        # Send goal to ROS action.
        send_goal_event = Event()

        def send_goal_callback(future):
            nonlocal send_goal_event
            send_goal_event.set()

        send_goal_future = client.send_goal_async(goal)

        send_goal_future.add_done_callback(send_goal_callback)
        send_goal_event.wait()

        goal_handle = send_goal_future.result()
        if not goal_handle.accepted:
            self.get_logger().info('Gather EEG goal rejected.')
            return None

        # Get result from ROS action.
        get_result_future = goal_handle.get_result_async()

        def get_result_done_callback(future):
            nonlocal gathered_eeg_buffer

            result = future.result()
            if result is None:
                self.get_logger().info('Gather EEG result failed.')
                return None

            eeg_buffer = result.result.eeg_buffer
            error = result.result.error

            gathered_eeg_buffer.eeg_buffer = eeg_buffer
            gathered_eeg_buffer.error = error

            if error.value != GatherEegError.NO_ERROR:
                self.get_logger().warn('{}: Failure: Error while gathering data from {:.3f} to {:.3f} (s), error code: {}.'.format(goal_id, start_time, end_time, error.value))

            gathered_eeg_buffer.set()

        get_result_future.add_done_callback(get_result_done_callback)

    def gather_eeg_async(self, goal_id, baseline_time, time_window, gathered_eeg_buffer):
        start_time = baseline_time + time_window.start
        end_time = baseline_time + time_window.end

        self.call_action__gather_eeg__async(
            goal_id=goal_id,
            start_time=start_time,
            end_time=end_time,
            gathered_eeg_buffer=gathered_eeg_buffer,
        )

    # Logging

    def log_mep_configuration(self, goal_id, mep_configuration):
        time_window = mep_configuration.time_window

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Configuration: MEP time window: {:.1f} to {:.1f} (ms) after stimulation.'.format(goal_id, 1000 * time_window.start, 1000 * time_window.end))

        preactivation_check = mep_configuration.preactivation_check
        self.logger.info('{}: Configuration: Preactivation check {}.'.format(goal_id, "enabled" if preactivation_check.enabled else "disabled"))

        if preactivation_check.enabled:
            time_window = preactivation_check.time_window
            self.logger.info('{}: Configuration: Preactivation time window: {:.1f} to {:.1f} (ms) before stimulation.'.format(goal_id, 1000 * (-time_window.end), 1000 * (-time_window.start)))

        self.logger.info('{}:'.format(goal_id))

    # MEP analysis

    def validate_emg_channel(self, goal_id, emg_channel, eeg_buffer):

        # Assuming that all datapoints have the same channel count.
        emg_channel_count = len(eeg_buffer[0].emg_channels)

        if emg_channel > emg_channel_count:
            self.logger.warn('{}: Failure: Requested channel ({}) larger than channel count ({}).'.format(
                goal_id,
                emg_channel,
                emg_channel_count,
            ))
            return MepError(value=MepError.INVALID_EMG_CHANNEL)

        if emg_channel == 0:
            self.logger.warn('{}: Failure: Requested channel is zero, the channel numbering starts from 1.'.format(
                goal_id,
            ))
            return MepError(value=MepError.INVALID_EMG_CHANNEL)
        
        return MepError(value=MepError.NO_ERROR)

    def get_channel_from_eeg_buffer(self, emg_channel, eeg_buffer):

        # EMG channel numbering starts from 1, therefore decrement when indexing the array.
        channel_data = np.array([d.emg_channels[emg_channel - 1] for d in eeg_buffer])
        timestamps = np.array([d.time for d in eeg_buffer])

        return channel_data, timestamps

    def check_preactivation(self, goal_id, mep_configuration, emg_channel, preactivation_buffer):

        channel_data, timestamps = self.get_channel_from_eeg_buffer(
            emg_channel=emg_channel,
            eeg_buffer=preactivation_buffer,
        )

        min_value = np.min(channel_data)
        max_value = np.max(channel_data)

        voltage_range = mep_configuration.preactivation_check.voltage_range

        min_allowed = voltage_range.low
        max_allowed = voltage_range.high

        if min_value < min_allowed:
           self.logger.warn('{}: Failure: Preactivation check failed, found value {:.1f}, minimum allowed: {:.1f}.'.format(goal_id, min_value, min_allowed))
           return MepError(value=MepError.PREACTIVATION_FAILED)

        if max_value > max_allowed:
           self.logger.warn('{}: Failure: Preactivation check failed, found value {:.1f}, maximum allowed: {:.1f}.'.format(goal_id, max_value, max_allowed))
           return MepError(value=MepError.PREACTIVATION_FAILED)

        self.logger.info('{}: Preactivation check passed.'.format(goal_id))

        return MepError(value=MepError.NO_ERROR)

    def compute_mep_amplitude_and_latency(self, goal_id, time, emg_channel, mep_buffer):

        channel_data, timestamps = self.get_channel_from_eeg_buffer(
            emg_channel=emg_channel,
            eeg_buffer=mep_buffer,
        )
        max_i = np.argmax(channel_data)
        min_i = np.argmin(channel_data)

        amplitude = channel_data[max_i] - channel_data[min_i]
        latency = timestamps[max_i] - time

        self.logger.info('{}: Computed MEP with amplitude {:.1f} (uV) and latency {:.1f} (ms).'.format(goal_id, amplitude, 1000 * latency))

        return amplitude, latency

    def analyze_mep(self, goal_id, emg_channel, time, mep_configuration):
        self.logger.info('{}: Analyzing MEP at time: {:.3f} (s)'.format(goal_id, time))

        self.log_mep_configuration(goal_id, mep_configuration)

        errors = AnalyzeMepErrors()

        gathered_preactivation_buffer = GatheredEegBuffer()
        gathered_mep_buffer = GatheredEegBuffer()

        preactivation_check_enabled = mep_configuration.preactivation_check.enabled

        # Gather preactivation data asynchronously.

        if preactivation_check_enabled:
            self.logger.info('{}: Gathering preactivation data...'.format(goal_id))

            self.gather_eeg_async(
                goal_id=goal_id,
                baseline_time=time,
                time_window=mep_configuration.preactivation_check.time_window,
                gathered_eeg_buffer=gathered_preactivation_buffer,
            )

        # Gather MEP data asynchronously.

        self.logger.info('{}: Gathering MEP data...'.format(goal_id))

        self.gather_eeg_async(
            goal_id=goal_id,
            baseline_time=time,
            time_window=mep_configuration.time_window,
            gathered_eeg_buffer=gathered_mep_buffer,
        )

        # Wait for gathering to be finished.

        if preactivation_check_enabled:
            gathered_preactivation_buffer.wait()

        gathered_mep_buffer.wait()

        # Collect errors from gatherers.

        if preactivation_check_enabled:
            errors.gather_preactivation_error = gathered_preactivation_buffer.error

        errors.gather_mep_error = gathered_mep_buffer.error

        # Combine and check errors before continuing.

        if errors.gather_mep_error.value != GatherEegError.NO_ERROR or errors.gather_preactivation_error.value != GatherEegError.NO_ERROR:
            errors.mep_error = MepError(value=MepError.GATHERING_DATA_FAILED)

        if errors.mep_error.value != MepError.NO_ERROR:
            amplitude = None
            latency = None

            return errors, amplitude, latency

        self.logger.info('{}: Finished gathering data.'.format(goal_id))

        # Validate EMG channel.

        mep_error = self.validate_emg_channel(
            goal_id=goal_id,
            emg_channel=emg_channel,
            eeg_buffer=gathered_mep_buffer.eeg_buffer,
        )
        errors.mep_error = mep_error

        if errors.mep_error.value != MepError.NO_ERROR:
            amplitude = None
            latency = None

            return errors, amplitude, latency

        # Check preactivation

        mep_error = self.check_preactivation(
            goal_id=goal_id,
            mep_configuration=mep_configuration,
            emg_channel=emg_channel,
            preactivation_buffer=gathered_preactivation_buffer.eeg_buffer,
        )
        errors.mep_error = mep_error

        if errors.mep_error.value != MepError.NO_ERROR:
            amplitude = None
            latency = None

            return errors, amplitude, latency

        # Compute MEP based on gathered data.

        amplitude, latency = self.compute_mep_amplitude_and_latency(
            goal_id=goal_id,
            time=time,
            emg_channel=emg_channel,
            mep_buffer=gathered_mep_buffer.eeg_buffer,
        )

        return errors, amplitude, latency

    def analyze_mep_action_handler(self, goal_handle):
        request = goal_handle.request

        emg_channel = request.emg_channel
        time = request.time
        mep_configuration = request.mep_configuration

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        errors, amplitude, latency = self.analyze_mep(
            goal_id=goal_id,
            emg_channel=emg_channel,
            time=time,
            mep_configuration=mep_configuration
        )

        # Create and return a Result object.

        result = AnalyzeMep.Result()

        goal_handle.succeed()

        # HACK: ROS doesn't seem to support NaNs in floats, work around it by encoding None as 0.0.
        result.amplitude = amplitude if amplitude is not None else 0.0
        result.latency = latency if latency is not None else 0.0
        result.errors = errors

        return result

    def analyze_mep_service_handler(self, request, response):
        emg_channel = request.emg_channel
        time = request.time
        mep_configuration = request.mep_configuration

        # HACK: Service calls are not assigned an ID by ROS, therefore assign a constant ID here.
        #   It is used only as the prefix for the log messages.
        #
        goal_id = 'abcd'

        self.logger.info('{}: '.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        errors, amplitude, latency = self.analyze_mep(
            goal_id=goal_id,
            emg_channel=emg_channel,
            time=time,
            mep_configuration=mep_configuration
        )

        # HACK: ROS doesn't seem to support NaNs in floats, work around it by encoding None as 0.0.
        response.amplitude = amplitude if amplitude is not None else 0.0
        response.latency = latency if latency is not None else 0.0
        response.error = errors

        return response


def main(args=None):
    rclpy.init(args=args)

    analyze_mep_node = AnalyzeMepNode()

    # Allow several actions to be executed concurrently.
    #
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(analyze_mep_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
