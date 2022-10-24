import rclpy
from rclpy.node import Node
import time

from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendSignalOut, StartDevice, StartExperiment, StopExperiment, \
    SendPulse, SendCharge
from fpga_interfaces.msg import SignalOut, Event

TRIGGER_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
EVENT_ID = 1
TIME_CONSTANT_US = int(1e6) * 1


class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_trigger_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received',
                                                           self.trigger_reader_callback, 10)

        self.trigger_client = self.create_client(SendSignalOut, '/fpga/send_signal_out')
        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        self.request = SendSignalOut.Request()
        self.stimulation_request = SendPulse.Request()
        self.charge_request = SendCharge.Request()

        self.first_trigger_time = 0
        self.last_trigger_time = 0

        self.client_futures = []

        self.init_device()

    def init_device(self):
        self.restart_experiment()
        self.get_logger().info('Experiment started')

    def data_reader_callback(self, msg):
        pass

    def trigger_reader_callback(self, msg):
        if msg.index == 1:
            self.first_trigger_time = msg.time_us
            self.set_trigger_request(1, msg.time_us)
            self.client_futures.append(self.trigger_client.call_async(self.request))

        elif msg.index == 2:
            self.last_trigger_time = msg.time_us
            self.get_logger().info(f'Difference between triggers: {msg.time_us} us')
            self.log_to_file(self.last_trigger_time)

    def log_to_file(self, time_difference):
        with open('latencies.txt', 'a') as f:
            f.write(str(time_difference) + "\n")

    def set_trigger_request(self, index, time_us):
        event = Event()
        event.id = EVENT_ID
        event.execution_condition = 2
        event.time_us = time_us + TIME_CONSTANT_US

        signal_out = SignalOut()
        signal_out.port = port
        signal_out.duration_us = TRIGGER_DURATION_US
        signal_out.event = event

        self.request.signal_out = signal_out

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
