import rclpy
from rclpy.node import Node
import time

from event_interfaces.msg import TriggerOut, Pulse, Charge
from mtms_device_interfaces.srv import StartDevice, StartSession, StopSession

from .pulses import generate_standard_pulse_command, generate_standard_charge_command, generate_timed_pulses, generate_timed_charges, generate_trigger_out_command

TRIGGER_OUT_DURATION_US = 10000
SAMPLING_INTERVAL = 0.0002
TIME_CONSTANT = 1.0


class PulseGenerator(Node):
    def __init__(self):
        super().__init__('pulse_generator')
        self.trigger_out_publisher = self.create_publisher(TriggerOut, '/event/send/trigger_out')
        self.pulse_publisher = self.create_publisher(Pulse, '/event/send/pulse')
        self.charge_publisher = self.create_publisher(Charge, '/event/send/charge')

        self.start_device_client = self.create_client(StartDevice, '/mtms_device/start_device')
        self.start_session_client = self.create_client(StartSession, '/mtms_device/start_session')
        self.stop_session_client = self.create_client(StopSession, '/mtms_device/stop_session')

        self.client_futures = []

        self.pulse_sent_at = None

        self.restart_session()

        self.pulses = generate_standard_pulse_command()
        self.charges = generate_standard_charge_command(1200)
        self.trigger_out = generate_trigger_out_command(port=1, execution_time=0.0, execution_condition=2, duration=TRIGGER_OUT_DURATION_US)

        self.timed_pulses = generate_timed_pulses(100)
        self.timed_charges = generate_timed_charges(100, 1200)
        
        self.send_charges()

        time.sleep(3)

        self.create_timer(2, self.timer_callback)
        self.pulses_sent = 0

    def send_charges(self):
        for charge in self.charges:
            self.charge_publisher(charge)
            self.get_logger().info(f"Sent charge message for channel {charge.channel} {charge.target_voltage}V")

    def send_timed_charges(self):
        for charge in self.timed_charges:
            self.charge_publisher(charge)
            self.get_logger().info(f"Sent timed charge request for channel {charge.channel} {charge.target_voltage}V")
   
    def send_timed_pulse_commands(self):
        for pulse in self.timed_pulses:
            self.pulse_publisher(pulse)
            self.get_logger().info(f"Sent timed pulse request for channel {pulse.channel}: {pulse.event_info.id}")

    def timer_callback(self):
        self.trigger_out_publisher(self.trigger_out)
        self.get_logger().info(f"Sent trigger out for port {self.trigger_out.port}")

        for pulse in self.pulses:
            self.pulse_publisher(pulse)
            self.get_logger().info(f"Sent pulse request for channel {pulse.channel}")
            self.get_logger().info(f"Pulse count {self.pulses_sent}")
            self.pulses_sent += 1
        
        time.sleep(0.1)

        for charge in self.charges:
            self.charge_publisher(charge)
            self.get_logger().info(f"Sent charge request for channel {charge.channel} {charge.target_voltage}V")

    def restart_session(self):
        self.stop_session_client.call_async(StopSession.Request())
        time.sleep(0.1)
        self.start_session_client.call_async(StartSession.Request())

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
