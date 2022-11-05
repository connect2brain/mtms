import rclpy
from rclpy.node import Node
import time

from fpga_interfaces.srv import SendSignalOut, StartDevice, StartExperiment, StopExperiment, SendPulse, SendCharge

from .pulses import generate_standard_pulse_command, generate_standard_charge_command, generate_timed_pulses, generate_timed_charges, generate_signal_out_command

SIGNAL_OUT_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
TIME_CONSTANT = 1.0


class PulseGenerator(Node):
    def __init__(self):
        super().__init__('pulse_generator')
        self.signal_out_client = self.create_client(SendSignalOut, '/fpga/send_signal_out')
        self.pulse_client = self.create_client(SendPulse, '/fpga/send_pulse')
        self.charge_client = self.create_client(SendCharge, '/fpga/send_charge')
        self.start_device_client = self.create_client(StartDevice, '/fpga/start_device')
        self.start_experiment_client = self.create_client(StartExperiment, '/fpga/start_experiment')
        self.stop_experiment_client = self.create_client(StopExperiment, '/fpga/stop_experiment')

        self.signal_out_request = SendSignalOut.Request()
        self.pulse_request = SendPulse.Request()
        self.charge_request = SendCharge.Request()

        self.client_futures = []

        self.pulse_sent_at = None

        self.restart_experiment()

        self.pulses = generate_standard_pulse_command()
        self.charges = generate_standard_charge_command(1200)
        self.signal_out_request.signal_out = generate_signal_out_command(port=1, time=0.0, execution_condition=2, duration=SIGNAL_OUT_DURATION_US)

        self.timed_pulses = generate_timed_pulses(100)
        self.timed_charges = generate_timed_charges(100, 1200)
        
        self.send_charges()

        time.sleep(3)

        self.create_timer(2, self.timer_callback)
        self.pulses_sent = 0

    def send_charges(self):
        for charge in self.charges:
            self.charge_request.charge = charge
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent charge request for channel {charge.channel} {charge.target_voltage}V")

    def send_timed_charges(self):
        for charge in self.timed_charges:
            self.charge_request.charge = charge
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent timed charge request for channel {charge.channel} {charge.target_voltage}V")
   
    def send_timed_pulse_commands(self):
        for pulse in self.timed_pulses:
            self.pulse_request.pulse = pulse
            self.pulse_client.call_async(self.pulse_request)
            self.get_logger().info(f"Sent timed pulse request for channel {pulse.channel}: {pulse.event.id}")

    def timer_callback(self):
        self.signal_out_client.call_async(self.signal_out_request)
        self.get_logger().info(f"Sent signal out for port {self.signal_out_request.signal_out.port}")

        for pulse in self.pulses:
            self.pulse_request.pulse = pulse
            self.pulse_client.call_async(self.pulse_request)
            self.get_logger().info(f"Sent pulse request for channel {pulse.channel}")
            self.get_logger().info(f"Pulse count {self.pulses_sent}")
            self.pulses_sent += 1
        
        time.sleep(0.1)

        for charge in self.charges:
            self.charge_request.charge = charge
            self.charge_client.call_async(self.charge_request)
            self.get_logger().info(f"Sent charge request for channel {charge.channel} {charge.target_voltage}V")

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
