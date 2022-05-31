from mtms_interfaces.msg import StimulationPulsePiece

fullEvent = {
    'stimulation_pulse_event': {
        'pieces': [
            {'mode': 4, 'duration_in_ticks': 10000},
            {'mode': 3, 'duration_in_ticks': 30000},
            {'mode': 2, 'duration_in_ticks': 50000}
        ],
        'channel': 2,
        'event_info': {
            'event_id': 20,
            'wait_for_trigger': True,
            'time_us': 10000000,
            'delay_us': 0
        }
    }
}

_pulse = [
    {'mode': 4, 'duration_in_ticks': 10000},
    {'mode': 3, 'duration_in_ticks': 30000},
    {'mode': 2, 'duration_in_ticks': 50000}
]

pulses = {}
TICK_DURATION_IN_US = 25


def pulse_piece_from_dict(d):
    piece = StimulationPulsePiece()
    piece.mode = d["mode"]
    piece.duration_in_ticks = d["duration_in_ticks"]
    return piece


def generate_pulse(channel, voltage):
    return pulses[channel]


def pulse_length_in_us(pulse):
    s = 0
    for piece in pulse:
        s += piece.duration_in_ticks * TICK_DURATION_IN_US
    return s


for index in range(0, 6):
    pieces = list(map(lambda piece: pulse_piece_from_dict(piece), _pulse))
    pulses[index + 1] = pieces
