import rclpy
from rclpy.node import Node

from mtms_interfaces.srv import StartPulseSequence
from mtms_interfaces.msg import StimulationPulseEvent, EventInfo
from .pulses import generate_pulse

START_DELAY = 100


class StartPulseSequenceNode(Node):
    def __init__(self):
        super().__init__('start_pulse_sequence')
        self.create_service(StartPulseSequence, '/stimulation/start_experiment', self.start_pulse_sequence_callback)
        self.start_time = self.get_clock().now().nanoseconds / 1000

    def sequence_is_possible(self, sequence):
        return True

    # All times are relative to self.start_time, which is the time when the experiment command was received
    def calculate_time_us(self, train_interval, burst_interval, stimulus_interval):
        return int(self.start_time + START_DELAY + train_interval + burst_interval + stimulus_interval)

    def send_stimulation_pulse_event(self, event):
        self.get_logger().info("SENDING: " + str(event))

    def start_pulse_sequence_callback(self, request, response):
        self.get_logger().info('Incoming request')
        self.get_logger().info(str(request))
        pulse_sequence = request.experiment.pulse_sequence
        if not self.sequence_is_possible(pulse_sequence):
            response.success = False
            response.status = 'sequence is not possible'
            response.sequence = []
            return response

        event_id = 0
        train_interval = 0
        burst_interval = 0

        sequence = []

        for train_index in range(pulse_sequence.nof_trains):
            for burst_index in range(pulse_sequence.nof_bursts_in_trains):
                for pulse_index in range(pulse_sequence.nof_pulses_in_bursts):

                    # Send event for each enabled channel
                    for channel_info in pulse_sequence.channel_info:
                        channel_index = channel_info.channel_index
                        voltage = channel_info.voltage

                        pulse = generate_pulse(channel_index, voltage)

                        # isi for the first pulse is 0 as there are no preceding pulses to "wait" for
                        isi = 0 if pulse_index == 0 else pulse_sequence.isis[pulse_index - 1]

                        event_info = EventInfo()
                        event_info.event_id = event_id
                        event_info.wait_for_trigger = False
                        event_info.time_us = self.calculate_time_us(train_interval, burst_interval, isi)
                        event_info.delay_us = 0

                        stimulation_pulse_event = StimulationPulseEvent()
                        stimulation_pulse_event.pieces = pulse
                        stimulation_pulse_event.channel = channel_index
                        stimulation_pulse_event.event_info = event_info

                        self.send_stimulation_pulse_event(stimulation_pulse_event)
                        sequence.append(stimulation_pulse_event)

                        event_id += 1
                burst_interval += pulse_sequence.ibi

            train_interval += pulse_sequence.iti
        self.get_logger().info(str(sequence))
        response.success = True
        response.sequence = sequence

        return response


def main():
    rclpy.init()

    start_sequence_node = StartPulseSequenceNode()

    rclpy.spin(start_sequence_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
