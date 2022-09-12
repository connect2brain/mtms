import rclpy
from rclpy.node import Node
import time
import sys
from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendTriggerOutEvent, StartDevice, StartExperiment, StopExperiment, \
    SendStimulationPulseEvent, SendChargeEvent
from fpga_interfaces.msg import TriggerOutEvent, EventInfo

SAMPLING_FREQUENCY = 20000


class DataProvider(Node):

    def __init__(self):
        super().__init__('data_provider')
        self.publisher = self.create_publisher(EegDatapoint, '/eeg/raw_data', 10)
        self.declare_parameter('data_file', "")
        data_file_name = self.get_parameter('data_file').value
        self.get_logger().info(f"data file name {data_file_name}")
        self.file = open(data_file_name, 'r')
        self.create_timer(1 / SAMPLING_FREQUENCY, self.publish_data)

    def publish_data(self):
        data = [float(number) for number in self.file.readline().split(",")]
        msg = EegDatapoint()
        msg.channel_datapoint = data
        msg.first_sample_of_experiment = False
        msg.time = float(self.get_clock().now().nanoseconds)
        self.publisher.publish(msg)


def main():
    rclpy.init()
    data_provider = DataProvider()
    rclpy.spin(data_provider)
    rclpy.shutdown()


if __name__ == '__main__':
    main()
