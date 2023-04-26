def analyze_eeg(data):
    return True


def get_default_waveform(channel):
    falling_phase_duration_in_ticks = 1480
    if channel == 3 or channel == 4:
        falling_phase_duration_in_ticks = 1564
    elif channel == 5:
        falling_phase_duration_in_ticks = 1776

    return [
        {
            "waveform_phase": 0,
            "duration_in_ticks": 2400
        },
        {
            "waveform_phase": 1,
            "duration_in_ticks": 1200
        },
        {
            "waveform_phase": 2,
            "duration_in_ticks": falling_phase_duration_in_ticks
        }
    ]
