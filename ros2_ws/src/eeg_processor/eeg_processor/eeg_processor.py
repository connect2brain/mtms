import rclpy
from rclpy.node import Node
import time
from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendTriggerOutEvent, StartDevice, StartExperiment, StopExperiment
from fpga_interfaces.msg import TriggerOutEvent, EventInfo

TRIGGER_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
EVENT_ID = 1
TIME_CONSTANT_US = int(1e6) * 1
DELAY_US = 0

class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback, 10)
        
        self.trigger_client = self.create_client(SendTriggerOutEvent, '/fpga/send_trigger_out_event')
        self.start_device_client = self.create_client(StartDevice, '/fpga/start_device')
        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        self.request = SendTriggerOutEvent.Request()

        self.first_trigger_time = 0
        self.last_trigger_time = 0

        self.client_futures = []

        self.start_device_client.call_async(StartDevice.Request())
        # self.restart_experiment()

    def data_reader_callback(self, msg):
        pass
        # self.get_logger().info("Timestamp: {} ms. First sample of experiment {} \n".format(msg.time, msg.first_sample_of_experiment))


    def trigger_reader_callback(self, msg):

        if msg.index == 1:
            self.first_trigger_time = msg.time_us

            self.set_trigger_request(msg.index, msg.time_us)
            self.start_experiment_client.call_async(StartExperiment.Request())
            self.client_futures.append(self.trigger_client.call_async(self.request))

        elif msg.index == 2:
            self.stop_experiment_client.call_async(StopExperiment.Request())
            self.last_trigger_time = msg.time_us
            self.get_logger().info("Time difference between triggers: {}".format(self.last_trigger_time))
            self.log_to_file(self.last_trigger_time)

    def log_to_file(self, time_difference):
        with open('latencies.txt', 'a') as f:
            f.write(str(time_difference) + "\n")

    def set_trigger_request(self, index, time_us):
        self.get_logger().info(f"event info time_us = {time_us + TIME_CONSTANT_US}")
        event_info = EventInfo()
        event_info.event_id = EVENT_ID
        event_info.wait_for_trigger = False
        event_info.time_us = time_us + TIME_CONSTANT_US
        event_info.delay_us = DELAY_US

        trigger_event = TriggerOutEvent()
        trigger_event.index = 3
        trigger_event.duration_us = TRIGGER_DURATION_US
        trigger_event.event_info = event_info

        self.request.trigger_out_event = trigger_event


    def restart_experiment(self):
        self.stop_experiment_client.call_async(StopExperiment.Request())
        time.sleep(0.1)
        self.start_experiment_client.call_async(StartExperiment.Request())


    def spin(self):
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