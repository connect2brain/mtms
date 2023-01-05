import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import EegDatapoint, Trigger
from rcl_interfaces.msg import ParameterDescriptor, ParameterType


class DataProvider(Node):

    # TODO: Make configurable.
    #
    NUMBER_OF_EEG_CHANNELS = 62

    def __init__(self):
        super().__init__('eeg_simulator')

        self.current_time = 0.0

        self.eeg_publisher = self.create_publisher(EegDatapoint, '/eeg/raw_data', 10)
        self.trigger_publisher = self.create_publisher(Trigger, '/eeg/trigger_received', 10)

        self.declare_parameter('data_file', "")

        self.data_file_name = self.get_parameter('data_file').value

        descriptor = ParameterDescriptor(
            name='Sampling frequency',
            type=ParameterType.PARAMETER_INTEGER,
        )
        self.declare_parameter('sampling_frequency', descriptor=descriptor)
        self.sampling_frequency = self.get_parameter('sampling_frequency').value

        self.sampling_period = 1 / self.sampling_frequency

        self.get_logger().info(f"data file name {self.data_file_name}")

        self.file = open(self.data_file_name, 'r')

        self.create_timer(self.sampling_period, self.publish_data)
        self.create_timer(5, self.publish_trigger)

    def publish_data(self):
        self.get_logger().info("Publishing data")

        line = self.file.readline()

        # if EOF, start from the beginning
        if line == "":
            self.file = open(self.data_file_name, 'r')
            line = self.file.readline()

        data = [float(number) for number in line.split(",")]

        msg = EegDatapoint()
        msg.eeg_channels = data[:self.NUMBER_OF_EEG_CHANNELS]
        msg.emg_channels = data[self.NUMBER_OF_EEG_CHANNELS:]
        msg.first_sample_of_experiment = False if self.current_time > 0 else True
        msg.time = self.current_time

        self.eeg_publisher.publish(msg)

        self.current_time += self.sampling_period

    def publish_trigger(self):
        self.get_logger().info("Publishing trigger")

        msg = Trigger()
        msg.index = 0
        msg.time = self.current_time

        self.trigger_publisher.publish(msg)


def main():
    rclpy.init()
    eeg_simulator = DataProvider()
    rclpy.spin(eeg_simulator)
    rclpy.shutdown()


if __name__ == '__main__':
    main()
