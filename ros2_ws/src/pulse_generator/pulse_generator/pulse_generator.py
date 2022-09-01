import rclpy
from rclpy.node import Node
import time

from fpga_interfaces.srv import SendTriggerOutEvent, StartDevice, StartExperiment, StopExperiment, SendStimulationPulseEvent, SendChargeEvent

from .pulses import generate_standard_pulse_command, generate_standard_charge_command, generate_timed_pulses, generate_timed_charges, generate_trigger_out_command

TRIGGER_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
EVENT_ID = 1
TIME_CONSTANT_US = int(1e6) * 1


class PulseGenerator(Node):
    def __init__(self):
        super().__init__('pulse_generator')
        self.trigger_client = self.create_client(SendTriggerOutEvent, '/fpga/send_trigger_out_event')
        self.stimulation_pulse_client = self.create_client(SendStimulationPulseEvent, '/fpga/send_stimulation_pulse_event')
        self.charge_client = self.create_client(SendChargeEvent, '/fpga/send_charge_event')
        self.start_device_client = self.create_client(StartDevice, '/fpga/start_device')
        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        self.trigger_request = SendTriggerOutEvent.Request()
        self.stimulation_request = SendStimulationPulseEvent.Request()
        self.charge_request = SendChargeEvent.Request()

        self.client_futures = []

        self.pulse_sent_at = None

        self.restart_experiment()

        self.pulse_events = generate_standard_pulse_command()
        self.charge_events = generate_standard_charge_command(1200)
        self.trigger_request.trigger_out_event = generate_trigger_out_command(1, 0, 2, TRIGGER_DURATION_US)

        self.timed_pulse_events = generate_timed_pulses(100)
        self.timed_charge_events = generate_timed_charges(100, 1200)
        
        self.send_charge_events()

        time.sleep(3)

        self.create_timer(2, self.timer_callback)
        self.pulses_sent = 0

    def send_charge_events(self):
        for event in self.charge_events:
            self.charge_request.charge_event = event
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent charge request for channel {event.channel} {event.target_voltage}V")

    def send_timed_charge_events(self):
        for event in self.timed_charge_events:
            self.charge_request.charge_event = event
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent timed charge request for channel {event.channel} {event.target_voltage}V")
   
    def send_timed_pulse_commands(self):
        for event in self.timed_pulse_events:
            self.stimulation_request.stimulation_pulse_event = event
            self.stimulation_pulse_client.call_async(self.stimulation_request)
            self.get_logger().info(f"Sent timed stimulation request for channel {event.channel}: {event.event_info.event_id}")

    def timer_callback(self):
        self.trigger_client.call_async(self.trigger_request)
        self.get_logger().info(f"Sent trigger out for index {self.trigger_request.trigger_out_event.index}")

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
    pulse_generator = PulseGenerator()
    pulse_generator.spin()

if __name__ == '__main__':
    main()