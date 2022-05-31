import rclpy
from rclpy.node import Node

from mtms_interfaces.srv import StartPulseSequence
from pulses import generate_pulse
from time import time_ns

START_DELAY = 100


class StartPulseSequenceNode(Node):
    def __init__(self):
        super().__init__('start_pulse_sequence')
        self.create_service(StartPulseSequence, '/stimulation/start_experiment', self.start_pulse_sequence)
        self.start_time = time_ns() / 1000

    def sequence_is_possible(self, sequence):
        return True

    # All times are relative to self.start_time, which is the time when the experiment command was received
    def calculate_time_us(self, train_interval, burst_interval, stimulus_interval):
        return self.start_time + START_DELAY + train_interval + burst_interval + stimulus_interval

    def start_pulse_sequence(self, request, response):
        self.get_logger().info('Incoming request')
        pulse_sequence = request.pulse_sequence
        if not self.sequence_is_possible(pulse_sequence):
            response.success = False
            return response

        event_id = 0
        train_interval = 0
        burst_interval = 0
        for train_index in range(pulse_sequence.nof_trains):
            for burst_index in range(pulse_sequence.nof_bursts_in_trains):
                for pulse_index in range(pulse_sequence.nof_pulses_in_bursts):

                    # Send command for each enabled channel
                    for channel_info in pulse_sequence.channel_info:
                        channel_index, voltage = channel_info
                        pulse = generate_pulse(channel_index, voltage)

                        isi = 0 if pulse_index == 0 else pulse_sequence.isis[pulse_sequence]

                        event_info = {
                            'event_id': event_id,
                            'wait_for_trigger': False,
                            'time_us': self.calculate_time_us(train_interval, burst_interval, isi),
                            'delay_us': 0
                        }

                        stimulation_pulse_event = {
                            'stimulation_pulse_event': {
                                'pieces': pulse,
                                'channel': channel_index,
                                'event_info': event_info
                            }
                        }
                        self.get_logger().info(str(stimulation_pulse_event))

                        event_id += 1
                burst_interval += pulse_sequence.ibi

            train_interval += pulse_sequence.iti

        response.success = True
        return response


def main():
    rclpy.init()

    start_sequence_node = StartPulseSequenceNode()

    rclpy.spin(start_sequence_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
