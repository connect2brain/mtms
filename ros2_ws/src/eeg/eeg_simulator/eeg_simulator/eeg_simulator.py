import socket
import struct
import os

import rclpy
from rclpy.node import Node

from eeg_interfaces.msg import EegDatapoint, EegInfo, Trigger
from rcl_interfaces.msg import ParameterDescriptor, ParameterType
from rclpy.qos import QoSProfile, DurabilityPolicy, HistoryPolicy, ReliabilityPolicy

UDP_IP = "127.0.0.1"
UDP_PORT = 50000
SCALE_FACTOR = 2**23

class EegDeviceSimulator():
    def __init__(self, udp_ip, port, data_path, sampling_rate, num_eeg_channels, loop, logger):
        self.ip = udp_ip
        self.port = port
        self.data_path = data_path
        self.loop = loop
        self.logger = logger  # TODO: remove
        self.sampling_rate = sampling_rate
        self.num_of_eeg_channels = num_eeg_channels
        self.time = 0.0
        self.sampling_period = 1 / self.sampling_rate

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sequence_number = 0

        # Initiate bit representations of constant fields for packets
        self.frame_type = 2     # uint8, 2 = Samples
        self.main_unit_num = 0  # uint8, 0 = Stand-alone
        self.reserved = [0,0]   # uint8[2]
        self.num_bundles = [(1 >> i) & 0xFF for i in (8, 0)]             # uint16 field
        self.num_of_channels_bits = [(self.num_of_eeg_channels >> i) & 0xFF for i in (8, 0)] # uint16 field
        self.first_sample_idx = [(28 >> i) & 0xFF for i in (56, 48, 40, 32, 24, 16, 8, 0)] # uint64 field

        self.send_measurement_start_packet()
        self.file = open(self.data_path, 'r')

    def send_measurement_start_packet(self):
        frame_type = 1                                                             # uint8 field, 1 = MeasurementStart
        sampling_rate = [(self.sampling_rate >> i) & 0xFF for i in (24, 16, 8, 0)] # uint32 field
        sample_format = [(0x80000018 >> i) & 0xFF for i in (24, 16, 8, 0)]         # uint32 field, fixed value
        trigger_defs = [(0 >> i) & 0xFF for i in (24, 16, 8, 0)]                   # uint32 field

        # TODO: Figure out what values these should actually be
        source_channels = []
        for i in range(self.num_of_eeg_channels):
            packed = struct.pack('2B', (0 >> 8) & 0xFF, 0 & 0xFF)
            source_channels.append(packed)
        source_channels = b''.join(source_channels)

        channel_types = [0b00101000 for i in range(self.num_of_eeg_channels)]
        total_bits = 3*self.num_of_eeg_channels + 18
        measurement_start_packet = struct.pack('{}B'.format(total_bits), frame_type, self.main_unit_num, self.reserved[0],
                                               self.reserved[1], *sampling_rate, *sample_format, *trigger_defs,
                                               *self.num_of_channels_bits, *source_channels, *channel_types)

        self.logger.info("Sending measurement start packet")
        self.sock.sendto(measurement_start_packet, (self.ip, self.port))

    def send_sample_packet(self):
        line = self.file.readline()

        if line == "" and self.loop:
            self.file = open(self.data_path, 'r')
            line = self.file.readline()

        elif line == "" and not self.loop:
            self.get_logger().info("Published all samples from file")

        # define the EEG data samples
        eeg_samples = [float(number) for number in line.split(",")]

        packet_seq_no = [(self.sequence_number >> i) & 0xFF for i in (24, 16, 8, 0)]  # uint32
        self.sequence_number += 1

        timestamp = [(0 >> i) & 0xFF for i in (56, 48, 40, 32, 24, 16, 8, 0)]         # uint64 TODO: fix correct value

        packed_samples = []
        for sample in eeg_samples[:self.num_of_eeg_channels]:
            # convert the floating-point value to fixed-point representation
            fixed_point = round(sample * SCALE_FACTOR)

            # pack the fixed-point value as 3 bytes in network byte order
            packed = struct.pack('!3B', (fixed_point >> 16) & 0xFF, (fixed_point >> 8) & 0xFF, fixed_point & 0xFF)

            # append the packed bytes to the list
            packed_samples.append(packed)

        packed_samples = b''.join(packed_samples)
        sample_package = struct.pack('{}B'.format(len(packed_samples)+28), self.frame_type, self.main_unit_num, self.reserved[0],
                                       self.reserved[1], *packet_seq_no, *self.num_of_channels_bits,
                                      *self.num_bundles, *self.first_sample_idx, *timestamp, *packed_samples)

        self.logger.info("Sending sample packet, timestamp: {:.2f}s, seqnum {}".format(self.time, self.sequence_number))
        self.time += self.sampling_period

        self.sock.sendto(sample_package, (self.ip, self.port))

class DataProvider(Node):

    EEG_RAW_TOPIC = '/eeg/raw'
    EEG_INFO_TOPIC = '/eeg/info'
    EEG_TRIGGER_RECEIVED_TOPIC = '/eeg/trigger_received'
    SIMULATE_EEG_DEVICE = True
    DATA_DIRECTORY = 'data/eeg/'

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

        descriptor = ParameterDescriptor(
            name='Simulate EEG device',
            type=ParameterType.PARAMETER_BOOL
        )
        self.declare_parameter('simulate_eeg_device', descriptor=descriptor)
        self.simulate_eeg_device = self.get_parameter('simulate_eeg_device').value

        self.get_logger().info(f"Reading data from file {self.data_file_name} in directory {self.DATA_DIRECTORY}.")

        self.data_path = os.path.join(self.DATA_DIRECTORY, self.data_file_name)

        if self.simulate_eeg_device:
            self.simulated_eeg_device = EegDeviceSimulator(UDP_IP, UDP_PORT, self.data_path, self.sampling_frequency,
                                                           self.eeg_channels, self.loop, self.get_logger())
            self.create_timer(self.sampling_period, self.simulated_eeg_device.send_sample_packet)

        else:
            self.file = open(self.data_path, 'r')
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
            self.file = open(self.data_path, 'r')
            line = self.file.readline()

        elif line == "" and not self.loop:
            self.get_logger().info("Published all samples from file")

        data = [float(number) for number in line.split(",")]
        assert self.total_channels <= len(data), "Total # of EEG and EMG channels ({}) exceeds # of channels in data ({})".format(self.total_channels, len(data))

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
