from threading import Thread
import time

import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendTriggerOutPulseEvent
from fpga_interfaces.msg.Event import TriggerOutPulseEvent

TRIGGER_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002

class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback, 10)

        self.data_subscriber  # prevent unused variable warning
        self.trigger_subscriber

        self.trigger_requested = False

        self.first_trigger_time = 0
        self.last_trigger_time = 0
        self.last_trigger_index = 0



    def data_reader_callback(self, msg):

#        for i in range(0,len(msg.channel_datapoint)):
#            self.get_logger().info("Channel {}: {}".format(i+1, msg.channel_datapoint[i]))

        self.get_logger().info("Timestamp: {} ms. First sample of experiment {} \n".format(msg.time, msg.first_sample_of_experiment))


    def trigger_reader_callback(self, msg):

        if msg.index == 1:
            self.first_trigger_time = msg.time_us
            self.last_trigger_index = 1 # <---- MAYBE REMOVE THIS?
            self.trigger_requested = True
            #self.send_trigger_request(msg.index)


        elif msg.index == 2:
            self.last_trigger_time = msg.time_us
            self.last_trigger_index = 2 # <---- MAYBE REMOVE THIS?
            self.get_logger().info("Time difference between triggers: {}".format(self.last_trigger_time))



class Connection(Thread):

    def __init__(self):
        Thread.__init__(self)
        self.daemon = True

        self.eeg_processor = EegProcessor()

    def run(self):
        rclpy.spin(self.eeg_processor)
        rclpy.shutdown()


class TriggerProcessor(Node):

    def __init__(self):
        super().__init__('trigger_processor')
        self.trigger_client = self.create_client(SendTriggerOutPulseEvent, '/fpga/send_trigger_out_pulse_event')
        self.trigger_pulse_service = self.create_service(SendTriggerOutPulseEvent, '/fpga/send_trigger_out_pulse_event', self.send_trigger_out)

        self.request = SendTriggerOutPulseEvent.Request()


    def send_trigger_out(self, request, response=False):
        self.get_logger().info("\nSUCCESSFUL SERVICE CALL\n")
        return True

        """
        trigger_request = TriggerOutPulseEvent.Request()
        trigger_request.index = index
        trigger_request.duration_us = TRIGGER_DURATION_US
        # TODO: Implement EventInfo

        self.send_trigger_pulse.call_service(trigger_request)
        """


    def send_trigger_request(self, index):
        trigger_event = TriggerOutPulseEvent()
        trigger_event.index = index
        trigger_event.duration_us = TRIGGER_DURATION_US
        # TODO: Implement EventInfo

        self.request = trigger_event
        return self.trigger_client.call(self.request)


def main():
    rclpy.init()
    
    connection = Connection()
    connection.start()

    trigger_processor = TriggerProcessor()
    while True:

        while not connection.eeg_processor.trigger_requested:
            pass

        response = trigger_processor.send_trigger_request(connection.eeg_processor.last_trigger_index)
        print("\nTrigger sent: {}\n".format(response))


if __name__ == '__main__':
    main()