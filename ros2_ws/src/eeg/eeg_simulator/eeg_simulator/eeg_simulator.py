import rclpy
from rclpy.node import Node

from eeg_interfaces.msg import EegDatapoint, EegInfo, Trigger
from rcl_interfaces.msg import ParameterDescriptor, ParameterType
from rclpy.qos import QoSProfile, DurabilityPolicy, HistoryPolicy, ReliabilityPolicy


class DataProvider(Node):

    EEG_RAW_TOPIC = '/eeg/raw_data'
    EEG_INFO_TOPIC = '/eeg/info'
    EEG_TRIGGER_RECEIVED_TOPIC = '/eeg/trigger_received'

    def __init__(self):
        super().__init__('eeg_simulator')

        self.current_time = 0.0

        qos_persist_latest = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            reliability=ReliabilityPolicy.RELIABLE,
        )

        self.eeg_publisher = self.create_publisher(EegDatapoint, self.EEG_RAW_TOPIC, 10)
        self.trigger_publisher = self.create_publisher(Trigger, self.EEG_TRIGGER_RECEIVED_TOPIC, 10)
        self.eeg_info_publisher = self.create_publisher(EegInfo, self.EEG_INFO_TOPIC, qos_persist_latest)

        self.declare_parameter('data_file', "")

        self.data_file_name = self.get_parameter('data_file').value

        descriptor = ParameterDescriptor(
            name='Sampling frequency',
            type=ParameterType.PARAMETER_INTEGER,
        )
        self.declare_parameter('sampling_frequency', descriptor=descriptor)
        self.sampling_frequency = self.get_parameter('sampling_frequency').value

        self.sampling_period = 1 / self.sampling_frequency

        descriptor = ParameterDescriptor(
            name='Loop',
            type=ParameterType.PARAMETER_BOOL
        )
        self.declare_parameter('loop', descriptor=descriptor)
        self.loop = self.get_parameter('loop').value

        descriptor = ParameterDescriptor(
            name='Number of EEG channels',
            type=ParameterType.PARAMETER_INTEGER
        )
        self.declare_parameter('eeg_channels', descriptor=descriptor)
        self.eeg_channels = self.get_parameter('eeg_channels').value

        descriptor = ParameterDescriptor(
            name='Number of EMG channels',
            type=ParameterType.PARAMETER_INTEGER
        )
        self.declare_parameter('emg_channels', descriptor=descriptor)
        self.emg_channels = self.get_parameter('emg_channels').value
        self.total_channels = self.eeg_channels + self.emg_channels

        self.get_logger().info(f"Reading data from file: {self.data_file_name}")

        self.file = open(self.data_file_name, 'r')

        self.create_timer(self.sampling_period, self.publish_data)
        self.create_timer(5, self.publish_trigger)

        # Publish EegInfo.

        eeg_info = EegInfo()
        eeg_info.sampling_frequency = self.sampling_frequency
        eeg_info.n_channels = self.eeg_channels
        eeg_info.send_trigger_as_channel = False

        self.eeg_info_publisher.publish(eeg_info)

    def publish_data(self):
        line = self.file.readline()

        # if EOF, start from the beginning
        if line == "" and self.loop:
            self.file = open(self.data_file_name, 'r')
            line = self.file.readline()

        elif line == "" and not self.loop:
            self.get_logger().info("Published all samples from file")

        data = [float(number) for number in line.split(",")]
        assert self.total_channels < len(data), "Number of channels exceeds {}".format(len(data))

        msg = EegDatapoint()
        msg.eeg_channels = data[:self.eeg_channels]
        msg.emg_channels = data[self.eeg_channels:self.total_channels]
        msg.first_sample_of_experiment = False if self.current_time > 0 else True
        msg.time = self.current_time

        self.eeg_publisher.publish(msg)
        self.get_logger().info("Published EEG datapoint in topic {} with timestamp {:.2f} s.".format(self.EEG_RAW_TOPIC, self.current_time))

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
