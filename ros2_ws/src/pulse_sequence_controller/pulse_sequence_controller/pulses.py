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

pulse = [
    {'mode': 4, 'duration_in_ticks': 10000},
    {'mode': 3, 'duration_in_ticks': 30000},
    {'mode': 2, 'duration_in_ticks': 50000}
]

pulses = {}

for index in range(0, 6):
    pulses[index + 1] = pulse


def generate_pulse(channel, voltage):
    return pulses[channel]
