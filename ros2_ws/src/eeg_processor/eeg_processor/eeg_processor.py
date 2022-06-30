from threading import Thread
import time

import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import EegDatapoint, Trigger
#from fpga_interfaces.srv import SendTriggerOutPulseEvent
#from fpga_interfaces.msg.Event import TriggerOutPulseEvent

TRIGGER_DURATION_US = 10000

class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback, 10)
        #self.send_trigger_pulse = self.create_service(SendTriggerOutPulseEvent, 'fpga/send_trigger_out_pulse_event', self.send_trigger_out)

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
            self.send_trigger_request(msg.index)


        elif msg.index == 2:
            self.last_trigger_time = msg.time_us
            self.get_logger().info("Time difference between triggers: {}".format(self.last_trigger_time))


    def send_trigger_request(self, index):
            """
            trigger_request = TriggerOutPulseEvent.Request()
            trigger_request.index = index
            trigger_request.duration_us = TRIGGER_DURATION_US
            # TODO: Implement EventInfo

            self.send_trigger_pulse.call_service(trigger_request)
            """

    def send_trigger_out(self, request, response=False):
        
        self.get_logger().info("\nSUCCESSFUL SERVICE CALL\n")


class Connection(Thread):

    def __init__(self):
        Thread.__init__(self)
        self.daemon = True

        self.eeg_processor = EegProcessor()

    def run(self):
        rclpy.spin(self.eeg_processor)
        rclpy.shutdown()


def main():
    rclpy.init()
    
    connection = Connection()
    connection.start()

    while True:
        print("Testing loop")
        time.sleep(1)


if __name__ == '__main__':
    main()