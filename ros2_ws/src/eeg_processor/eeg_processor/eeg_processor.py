import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import 


class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback, 10)
        self.data_subscriber  # prevent unused variable warning
        self.trigger_subscriber

        self.first_trigger_time = 0
        self.last_trigger_time = 0

    def data_reader_callback(self, msg):

#        for i in range(0,len(msg.channel_datapoint)):
#            self.get_logger().info("Channel {}: {}".format(i+1, msg.channel_datapoint[i]))

        self.get_logger().info("Timestamp: {} ms. First sample of experiment {} \n".format(msg.time, msg.first_sample_of_experiment))

    def trigger_reader_callback(self, msg):
                
        if msg.index == 1:
            self.first_trigger_time = msg.time_us

        elif msg.index == 2:
            self.last_trigger_time = msg.time_us
            self.get_logger().info("Time difference between triggers: {}".format(self.last_trigger_time))
            

def main():
    rclpy.init()
    eeg_processor = EegProcessor()
    rclpy.spin(eeg_processor)
    rclpy.shutdown()


if __name__ == '__main__':
    main()