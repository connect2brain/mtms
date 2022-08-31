import rclpy
from rclpy.node import Node
import time
import gc
import numpy as np

from mtms_interfaces.msg import EegDatapoint, Trigger
from fpga_interfaces.srv import SendTriggerOutEvent, StartDevice, StartExperiment, StopExperiment, SendStimulationPulseEvent, SendChargeEvent
from fpga_interfaces.msg import TriggerOutEvent, EventInfo, SystemState, StimulationPulsePiece, StimulationPulseEvent

from .pulses import generate_standard_pulse_command, generate_standard_charge_command, generate_timed_pulses, generate_timed_charges

TRIGGER_DURATION_US = 100
SAMPLING_INTERVAL = 0.0002
EVENT_ID = 1
TIME_CONSTANT_US = int(1e6) * 1


class EegProcessor(Node):

    def __init__(self):
        super().__init__('eeg_processor')
        self.data_subscriber = self.create_subscription(EegDatapoint, '/eeg/raw_data', self.data_reader_callback, 10)
        self.trigger_subscriber = self.create_subscription(Trigger, '/eeg/trigger_received', self.trigger_reader_callback_pulse_artifact, 10)
        # self.system_state_subscriber = self.create_subscription(SystemState, '/fpga/system_state_monitor_state', self.system_state_callback, 10)

        self.trigger_client = self.create_client(SendTriggerOutEvent, '/fpga/send_trigger_out_event')
        self.stimulation_pulse_client = self.create_client(SendStimulationPulseEvent, '/fpga/send_stimulation_pulse_event')
        self.charge_client = self.create_client(SendChargeEvent, '/fpga/send_charge_event')
        self.start_device_client = self.create_client(StartDevice, '/fpga/start_device')
        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        # self.disable_checks_client = self.create_client(DisableChecks, '/fpga/disable_checks')

        self.request = SendTriggerOutEvent.Request()
        self.stimulation_request = SendStimulationPulseEvent.Request()
        self.charge_request = SendChargeEvent.Request()

        self.first_trigger_time = 0
        self.last_trigger_time = 0

        self.system_state = "Not set"

        self.client_futures = []

        self.pulse_sent_at = None
        self.artifact_detected = False

        self.pulse_events = generate_standard_pulse_command()
        self.timed_pulse_events = generate_timed_pulses(100)
        self.get_logger().info(f"{self.pulse_events[0]}")
        self.charge_events = generate_standard_charge_command(1200)
        self.timed_charge_events = generate_timed_charges(100, 1200)
        self.init_device()

        self.send_charge_events()
        #time.sleep(5)
        #self.send_timed_pulse_commands()
        #self.send_timed_charge_events()

        self.start_time = self.get_clock().now().nanoseconds / 1000

        time.sleep(3)

        self.create_timer(2, self.timer_callback)
        self.set_trigger_request(3, 0)
        self.pulses_sent = 0

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
        
        #if self.pulse_sent_at is not None:
        # self.log_to_file(new_data)

        if abs(new_data) > 20000 and not self.artifact_detected:
            #self.get_logger().info(f"Artifact diff from start time: {data_received_at - self.start_time}")
            if self.pulse_sent_at is None:
                self.get_logger().warn(f"Received TMS artifact before pulse_sent_at was set {new_data}")
                return

            diff = data_received_at - self.pulse_sent_at

            self.artifact_detected = True
            self.get_logger().info(f"Received TMS pulse artifact, time difference from pulse_sent_at: {diff} us")
            self.log_to_file(diff)

            self.send_charge_events()

    def system_state_callback(self, msg):
        self.get_logger().info(f'Received state msg {msg.state}')
        self.system_state = msg.state

    def wait_until_operational(self, timestep=0.1):
        while self.system_state != "Operational":
            self.get_logger().info('Waiting for system to be operational...')
            time.sleep(timestep)

    def send_charge_events(self):
        for event in self.charge_events:
            self.charge_request.charge_event = event
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent charge request for channel {event.channel} {event.target_voltage}V")

    def send_timed_charge_events(self):
        for event in self.timed_charge_events:
            self.charge_request.charge_event = event
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent charge request for channel {event.channel} {event.target_voltage}V")
   

    def send_timed_pulse_commands(self):
        for event in self.timed_pulse_events:
            self.stimulation_request.stimulation_pulse_event = event
            self.stimulation_pulse_client.call_async(self.stimulation_request)
            self.get_logger().info(f"Sent stimulation request for channel {event.channel}: {event.event_info.event_id}")


    def timer_callback(self):
        self.trigger_client.call_async(self.request)
        self.get_logger().info(f"Sent trigger out for index {self.request.trigger_out_event.index}")

        for event in self.pulse_events:
            self.stimulation_request.stimulation_pulse_event = event
            self.stimulation_pulse_client.call_async(self.stimulation_request)
            self.get_logger().info(f"Sent stimulation request for channel {event.channel}")
            self.get_logger().info(f"Pulse count {self.pulses_sent}")
            self.pulses_sent += 1
        
        time.sleep(0.1)

        for event in self.charge_events:
            self.charge_request.charge_event = event
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent charge request for channel {event.channel} {event.target_voltage}V")

    def trigger_reader_callback_pulse_artifact(self, msg):
        for event in self.pulse_events:
            self.stimulation_request.stimulation_pulse_event = event
            self.stimulation_pulse_client.call_async(self.stimulation_request)
            self.get_logger().info(f"Sent stimulation request for channel {event.channel}: {event}")
            time.sleep(0.1)
            self.send_charge_events()


        self.pulse_sent_at = self.get_clock().now().nanoseconds / 1000
        self.artifact_detected = False

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

        trigger_event = TriggerOutEvent()
        trigger_event.index = 1
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