import rclpy
from rclpy.node import Node
import time

from event_interfaces.msg import Pulse, Charge, SignalOut, EventInfo

from mtms_device_interfaces.srv import StartDevice, StartExperiment, StopExperiment

from eeg_interfaces.msg import EegDatapoint, Trigger

SIGNAL_OUT_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
EVENT_ID = 1
TIME_CONSTANT = 1.0


class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_trigger_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback, 10)

        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        self.send_signal_out_publisher = self.create_publisher(SignalOut, '/event/send/signal_out', 10)

        self.first_trigger_time = 0.0
        self.last_trigger_time = 0.0

        self.client_futures = []

        self.init_device()

    def init_device(self):
        self.restart_experiment()
        self.get_logger().info('Experiment started')

    def data_reader_callback(self, msg):
        pass

    def trigger_reader_callback(self, msg):
        if msg.index == 1:
            self.first_trigger_time = msg.time
            self.set_trigger_request(1, msg.time)
            self.client_futures.append(self.signal_out_client.call_async(self.request))

        elif msg.index == 2:
            self.last_trigger_time = msg.time
            self.get_logger().info(f'Difference between triggers: {msg.time} s')
            self.log_to_file(self.last_trigger_time)

    def log_to_file(self, time_difference):
        with open('latencies.txt', 'a') as f:
            f.write(str(time_difference) + "\n")

    def set_trigger_request(self, index, time):
        event_info = EventInfo()
        event_info.id = EVENT_ID
        event_info.execution_condition = 2
        event_info.execution_time = time + TIME_CONSTANT

        signal_out = SignalOut()
        signal_out.port = port
        signal_out.duration_us = SIGNAL_OUT_DURATION_US
        signal_out.event_info = event_info

        self.send_signal_out_publisher.publish(signal_out)

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
                    # print("\n Result is {}\n".format(result))
                    # self.get_logger().info("")

                else:
                    incomplete_futures.append(f)

            self.client_futures = incomplete_futures


def main():
    rclpy.init()
    eeg_processor = EegProcessor()
    eeg_processor.spin()


if __name__ == '__main__':
    main()
