from fpga_interfaces.msg import PulsePiece

fullEvent = {
    'pulse': {
        'pieces': [
            {'current_mode': 4, 'duration_in_ticks': 10000},
            {'current_mode': 3, 'duration_in_ticks': 30000},
            {'current_mode': 2, 'duration_in_ticks': 50000}
        ],
        'channel': 2,
        'event': {
            'id': 20,
            # TODO: Bitrotten: 'wait_for_trigger' has been replaced with 'execution_condition'.
            'wait_for_trigger': True,
            'time_us': 10000000,
            'delay_us': 0
        }
    }
}

_pulse = [
    {'current_mode': 4, 'duration_in_ticks': 10000},
    {'current_mode': 3, 'duration_in_ticks': 30000},
    {'current_mode': 2, 'duration_in_ticks': 50000}
]

pulses = {}
TICK_DURATION_IN_US = 25


def pulse_piece_from_dict(pulse_dict):
    piece = PulsePiece()
    piece.current_mode.value = pulse_dict['current_mode']
    piece.duration_in_ticks = pulse_dict['duration_in_ticks']
    return piece


def generate_pulse(channel, voltage):
    return pulses[channel]


def pulse_duration_in_us(pulse):
    duration = 0
    for piece in pulse:
        duration += piece.duration_in_ticks * TICK_DURATION_IN_US
    return duration / 1000


for index in range(0, 6):
    pieces = list(map(lambda piece: pulse_piece_from_dict(piece), _pulse))
    pulses[index + 1] = pieces
