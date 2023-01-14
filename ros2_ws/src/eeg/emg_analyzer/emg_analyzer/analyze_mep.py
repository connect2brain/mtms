import time

import numpy as np

from mtms_interfaces.action import AnalyzeMep
from mtms_interfaces.srv import AnalyzeMepService
from rcl_interfaces.msg import ParameterDescriptor, ParameterType

import rclpy
from rclpy.action import ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup

from .data_gatherer import DataGatherer


class AnalyzeMepNode(Node):

    def __init__(self):
        super().__init__('analyze_mep_node')

        descriptor = ParameterDescriptor(
            name='Sampling frequency',
            type=ParameterType.PARAMETER_INTEGER,
        )
        self.declare_parameter('sampling_frequency', descriptor=descriptor)
        self.sampling_frequency = self.get_parameter('sampling_frequency').value

        self.callback_group = ReentrantCallbackGroup()
        self.action_server = ActionServer(
            self,
            AnalyzeMep,
            '/emg/analyze_mep',
            self.analyze_mep_action_handler,
            callback_group=self.callback_group,
        )
        self.logger = self.get_logger()

        # HACK: MATLAB does not currently (version R2022b) support ROS2 actions. Work around
        #   it by creating a service, which produces the same output as the action.
        #
        self.analyze_mep_service = self.create_service(
            AnalyzeMepService,
            '/emg/analyze_mep_service',
            self.analyze_mep_service_handler,
            callback_group=self.callback_group,
        )

    def compute_mep_amplitude_and_latency(self, emg_buffer, time_buffer):
        max_i = np.argmax(emg_buffer)
        min_i = np.argmin(emg_buffer)

        amplitude = emg_buffer[max_i] - emg_buffer[min_i]
        latency = time_buffer[max_i]

        return amplitude, latency

    def analyze_mep(self, goal_id, emg_channel, start_time):
        self.logger.info('{}: Analyzing MEP starting at time: {:.2f} (s)'.format(goal_id, start_time))

        data_gatherer = DataGatherer(
            goal_id=goal_id,
            emg_channel=emg_channel,
            start_time=start_time,
            create_subscription=self.create_subscription,
            destroy_subscription=self.destroy_subscription,
            logger=self.logger,
            sampling_frequency=self.sampling_frequency,
        )

        while not data_gatherer.is_finished():
            # Do not check continuously if data gatherer is finished; instead sleep between checks.
            # Otherwise EMG subscriber seems to get buried under the load and starts dropping samples.
            #
            time.sleep(0.1)
            pass

        emg_buffer, time_buffer = data_gatherer.get_buffers()

        success = data_gatherer.success()

        if success:
            self.logger.info('{}: # of samples received: {}'.format(goal_id, len(emg_buffer)))

            amplitude, latency = self.compute_mep_amplitude_and_latency(emg_buffer, time_buffer)
        else:
            self.logger.info('{}: Failed to gather data.'.format(goal_id))

            amplitude = None
            latency = None

        data_gatherer.destroy()

        return success, amplitude, latency

    def analyze_mep_action_handler(self, goal_handle):
        request = goal_handle.request
        emg_channel = request.emg_channel
        start_time = request.time

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        success, amplitude, latency = self.analyze_mep(goal_id, emg_channel, start_time)

        # Create and return a Result object.

        result = AnalyzeMep.Result()

        if success:
            goal_handle.succeed()

            result.amplitude = amplitude
            result.latency = latency
        else:
            goal_handle.abort()

        return result

    def analyze_mep_service_handler(self, request, response):
        emg_channel = request.emg_channel
        start_time = request.time

        # HACK: Service calls are not assigned an ID by ROS, therefore assign a constant ID here.
        #   It is used only as the prefix for the log messages.
        #
        goal_id = 'abcd'

        success, amplitude, latency = self.analyze_mep(goal_id, emg_channel, start_time)

        if success:
            response.amplitude = amplitude
            response.latency = latency

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
