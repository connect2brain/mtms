import rclpy
from rclpy.node import Node
from mtms_interfaces.msg import EegDatapoint

DEFAULT_SAMPLING_FREQUENCY = 5000.0


class DataProvider(Node):

    def __init__(self):
        super().__init__('data_provider')
        self.publisher = self.create_publisher(EegDatapoint, '/eeg/raw_data', 10)

        self.declare_parameter('data_file', "")
        self.data_file_name = self.get_parameter('data_file').value

        self.declare_parameter('sampling_frequency', DEFAULT_SAMPLING_FREQUENCY)
        sampling_frequency = self.get_parameter('sampling_frequency').value

        self.get_logger().info(f"data file name {self.data_file_name}")

        self.file = open(self.data_file_name, 'r')

        self.create_timer(1 / sampling_frequency, self.publish_data)

    def publish_data(self):
        self.get_logger().info("Publishing data")

        line = self.file.readline()

        # if EOF, start from the beginning
        if line == "":
            self.file = open(self.data_file_name, 'r')
            line = self.file.readline()

        data = [float(number) for number in line.split(",")]
        msg = EegDatapoint()
        msg.channel_datapoint = data[:62]
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
