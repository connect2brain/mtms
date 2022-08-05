import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendTriggerOutEvent
from fpga_interfaces.msg import TriggerOutEvent, EventInfo

TRIGGER_DURATION_US = 1000000
SAMPLING_INTERVAL = 0.0002
TIME_CONSTANT_US = 0
DELAY_US = 0

class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback, 10)
        self.trigger_client = self.create_client(SendTriggerOutEvent, '/fpga/send_trigger_out_event')
        # self.trigger_service = self.create_service(SendTriggerOutEvent, '/fpga/send_trigger_out_event', self.send_trigger_out)

        self.request = SendTriggerOutEvent.Request()

        self.first_trigger_time = 0
        self.last_trigger_time = 0
        self.event_id = 0

        self.client_futures = []


    def data_reader_callback(self, msg):
        pass
        # self.get_logger().info("Timestamp: {} ms. First sample of experiment {} \n".format(msg.time, msg.first_sample_of_experiment))


    def trigger_reader_callback(self, msg):
        self.get_logger().info(f"msg index: {msg.index}")
        if msg.index == 1:
            self.first_trigger_time = msg.time_us

            self.set_trigger_request(msg.index, msg.time_us)
            self.client_futures.append(self.trigger_client.call_async(self.request))

        elif msg.index == 2:
            self.last_trigger_time = msg.time_us
            self.get_logger().info("Time difference between triggers: {}".format(self.last_trigger_time)) 


    def set_trigger_request(self, index, time_us):
        time_us = int(1e6) * 40
        event_info = EventInfo()
        event_info.event_id = self.event_id
        event_info.wait_for_trigger = False
        event_info.time_us = time_us + TIME_CONSTANT_US
        event_info.delay_us = DELAY_US

        trigger_event = TriggerOutEvent()
        trigger_event.index = 3
        trigger_event.duration_us = TRIGGER_DURATION_US
        trigger_event.event_info = event_info

        self.request.trigger_out_event = trigger_event
        self.event_id += 1


    # def send_trigger_out(self, request, response):
    #     self.get_logger().info("SUCCESSFUL SERVICE CALL\n")

    #     response.success = True
    #     return response


    def spin(self):
        self.get_logger().info("Starting eeg processor")
        while rclpy.ok():
            rclpy.spin_once(self)

            incomplete_futures = []
            for f in self.client_futures:
                if f.done():
                    result = f.result()
                    print("\n Result is {}\n".format(result))

                else:
                    incomplete_futures.append(f)

            self.client_futures = incomplete_futures


def main():
    rclpy.init()
    eeg_processor = EegProcessor()
    eeg_processor.spin()

if __name__ == '__main__':
    main()