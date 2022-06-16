import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import EegDatapoint


class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.subscription = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.listener_callback, 10)
        self.subscription  # prevent unused variable warning
        self.data = []

    def listener_callback(self, msg):

        for i in range(0,len(msg.channel_datapoint)):
            self.get_logger().info("Channel {}: {}".format(i+1, msg.channel_datapoint[i]))

        self.get_logger().info("Timestamp: {}. First sample of experiment {} \n".format(msg.time, msg.first_sample_of_experiment))

def main():
    rclpy.init()
    eeg_processor = EegProcessor()
    rclpy.spin(eeg_processor)
    rclpy.shutdown()


if __name__ == '__main__':
    main()