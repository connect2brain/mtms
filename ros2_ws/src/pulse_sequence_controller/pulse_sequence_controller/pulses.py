pulse = {
    'pieces': [
        {'mode': 4, 'duration_in_ticks': 10000},
        {'mode': 3, 'duration_in_ticks': 30000},
        {'mode': 2, 'duration_in_ticks': 50000}
    ],
    'channel': 2
}

pulses = {}

for index in range(0, 6):
    pulses[index + 1] = pulse
    pulses[index]['channel'] = index + 1


def generate_pulse(channel, voltage):
    return pulses[channel]
