import time

import numpy as np

from mtms_interfaces.action import AnalyzeMep
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
            execute_callback=self.execute_callback,
            callback_group=self.callback_group,
        )
        self.logger = self.get_logger()

    # TODO: A placeholder for an actual MEP analysis algorithm.
    #
    def analyze_mep(self, emg_buffer, time_buffer):
        max_i = np.argmax(emg_buffer)

        peak_amplitude = emg_buffer[max_i]
        latency = time_buffer[max_i]

        return peak_amplitude, latency

    def execute_callback(self, goal_handle):
        request = goal_handle.request
        emg_channel = request.emg_channel
        start_time = request.time

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

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
            time.sleep(1.0)
            pass

        emg_buffer, time_buffer = data_gatherer.get_buffers()

        result = AnalyzeMep.Result()

        if data_gatherer.success():
            goal_handle.succeed()

            self.logger.info('{}: # of samples received: {}'.format(goal_id, len(emg_buffer)))

            peak_amplitude, latency = self.analyze_mep(emg_buffer, time_buffer)

            result.peak_amplitude = peak_amplitude
            result.latency = latency
        else:
            goal_handle.abort()

        data_gatherer.destroy()

        return result


def main(args=None):
    rclpy.init(args=args)

    analyze_mep_node = AnalyzeMepNode()

    # Allow several goals to be executed concurrently.
    #
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(analyze_mep_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
