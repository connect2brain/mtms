import rclpy
from rclpy.node import Node
import time
import gc
import numpy as np

from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendTriggerOutEvent, StartDevice, StartExperiment, StopExperiment, SendStimulationPulseEvent
from fpga_interfaces.msg import TriggerOutEvent, EventInfo, SystemState, StimulationPulsePiece, StimulationPulseEvent

TRIGGER_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
EVENT_ID = 1
TIME_CONSTANT_US = int(1e6) * 1
DELAY_US = 0

pulse_event = {
    'stimulation_pulse_event': {
        'pieces': [
            {'mode': 4, 'duration_in_ticks': 10000},
            {'mode': 3, 'duration_in_ticks': 30000},
            {'mode': 2, 'duration_in_ticks': 50000}
        ],
        'channel': 2,
        'event_info': {
            'event_id': 1,
            'execution_cond': 2,
            'time_us': 10000000,
            'delay_us': 0
        }
    }
}

class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback_pulse_artefact, 10)
        # self.system_state_subscriber = self.create_subscription(SystemState, '/fpga/system_state_monitor_state', self.system_state_callback, 10)

        self.trigger_client = self.create_client(SendTriggerOutEvent, '/fpga/send_trigger_out_event')
        self.stimulation_pulse_client = self.create_client(SendStimulationPulseEvent, '/fpga/send_trigger_out_event')
        self.start_device_client = self.create_client(StartDevice, '/fpga/start_device')
        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        # self.disable_checks_client = self.create_client(DisableChecks, '/fpga/disable_checks')

        self.request = SendTriggerOutEvent.Request()
        self.stimulation_request = SendStimulationPulseEvent.Request()

        self.first_trigger_time = 0
        self.last_trigger_time = 0

        self.system_state = "Not set"

        self.client_futures = []

        self.eeg_data_mean = 0
        self.eeg_data = []
        self.eeg_length = 20
        self.eeg_threshold = 250
        self.pulse_sent_at = None

        self.init_device()

        if gc.isenabled():
            gc.disable()

    def eeg_mean(self):
        return np.mean(self.eeg_data)

    def init_device(self):
        # req = DisableChecks.Request()
        # req.enabled = True
        # self.disable_checks_client.call_async(req)

        # time.sleep(1)

        # self.start_device_client.call_async(StartDevice.Request())
        # self.wait_until_operational(1)

        self.restart_experiment()
        self.get_logger().info('Experiment started')

    def data_reader_callback(self, msg):
        data_received_at = self.get_clock().now().nanoseconds / 1000
        new_data = msg.channel_datapoint[4]
        self.eeg_data.append(new_data)
        if self.eeg_data > self.eeg_length:
            self.eeg_data.pop(0)

        self.eeg_data_mean = self.eeg_mean()

        if abs(new_data - self.eeg_data_mean) > self.eeg_threshold:
            if self.pulse_sent_at is None:
                self.get_logger().info("Received TMS artefact before pulse_sent_at was set ")
                return

            diff = data_received_at - self.pulse_sent_at
            self.get_logger().info(f"Received TMS pulse artefact, time difference from pulse_sent_at: {diff} us")
            self.log_to_file(diff)

    def system_state_callback(self, msg):
        self.get_logger().info(f'Received state msg {msg.state}')
        self.system_state = msg.state

    def wait_until_operational(self, timestep=0.1):
        while self.system_state != "Operational":
            self.get_logger().info('Waiting for system to be operational...')
            time.sleep(timestep)

    def trigger_reader_callback_pulse_artefact(self, msg):
        event_info = EventInfo()
        event_info.event_id = 1
        event_info.execution_condition = 2
        event_info.time_us = 0
        event_info.delay_us = DELAY_US

        piece1 = StimulationPulsePiece()
        piece1.duration_in_ticks = 10000
        piece1.mode = 4

        piece2 = StimulationPulsePiece()
        piece2.duration_in_ticks = 30000
        piece2.mode = 3

        piece3 = StimulationPulsePiece()
        piece3.duration_in_ticks = 50000
        piece3.mode = 2

        event = StimulationPulseEvent()
        event.event_info = event_info
        event.channel = 5
        event.pieces = [
            piece1,
            piece2,
            piece3
        ]
        
        self.stimulation_request.stimulation_pulse_event = event

        self.stimulation_pulse_client.call_async(self.stimulation_request)
        self.pulse_sent_at = self.get_clock().now().nanoseconds / 1000


    def trigger_reader_callback(self, msg):
        if msg.index == 1:
            self.first_trigger_time = msg.time_us
            self.set_trigger_request(msg.index, msg.time_us)
            self.client_futures.append(self.trigger_client.call_async(self.request))

        elif msg.index == 2:
            self.last_trigger_time = msg.time_us
            self.get_logger().info(f'Difference between triggers: {msg.time_us} us')
            self.log_to_file(self.last_trigger_time)

    def log_to_file(self, time_difference):
        with open('latencies.txt', 'a') as f:
            f.write(str(time_difference) + "\n")

    def set_trigger_request(self, index, time_us):
        event_info = EventInfo()
        event_info.event_id = EVENT_ID
        event_info.execution_condition = 2
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