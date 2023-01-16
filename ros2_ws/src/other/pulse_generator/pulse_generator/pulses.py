from event_interfaces.msg import EventInfo, WaveformPiece, Pulse, Charge, SignalOut

US_TO_TICKS_CONVERSION_RATIO = 40

# Channel modes and times in order
mode_list = [
    [1, 2, 3],
    #[1, 2, 3],
    #[1, 2, 3],
    #[1, 2, 3],
    #[1, 2, 3]
]

time_list = [
    [60, 30, 37.0],
    #[60, 30, 37.0],
    #[60, 30, 39.1],
    #[60, 30, 39.1],
    #[60, 30, 44.4]
]

# Generate standard pulse command, values in us

def generate_signal_out_command(port, execution_time, execution_condition, duration):
    event_info = EventInfo()
    event_info.id = 1
    event_info.execution_condition.value = execution_condition
    event_info.execution_time = execution_time

    signal_out = SignalOut()
    signal_out.port = port
    signal_out.duration_us = duration
    signal_out.event_info = event_info

    return signal_out



def generate_standard_pulse_command():
    assert len(mode_list) == len(time_list)

    pulses = []
    for i, _ in enumerate(time_list):
        waveform = []
        for j, _ in enumerate(time_list[i]):
            piece = WaveformPiece()
            piece.mode = mode_list[i][j]
            piece.duration_in_ticks = int(time_list[i][j] * US_TO_TICKS_CONVERSION_RATIO)
            waveform.append(piece)

        event_info = EventInfo()
        event_info.id = i
        event_info.execution_condition.value = 2
        event_info.execution_time = 0.0

        pulse = Pulse()
        pulse.event_info = event_info
        pulse.channel = i + 1
        pulse.waveform = waveform

        pulses.append(pulse)

    return pulses

WAIT = 1e-05

def generate_timed_pulses(count):
    assert len(mode_list) == len(time_list)

    pulses = []
    for i in range(count):
        waveform = []
        for j, _ in enumerate(time_list[0]):
            piece = WaveformPiece()
            piece.mode = mode_list[0][j]
            piece.duration_in_ticks = int(time_list[0][j] * US_TO_TICKS_CONVERSION_RATIO)
            waveform.append(piece)

        event_info = EventInfo()
        event_info.id = i
        event_info.execution_condition.value = 0
        event_info.execution_time = 1.0 * i + WAIT

        pulse = Pulse()
        pulse.event_info = event_info
        pulse.channel = 1
        pulse.waveform = waveform

        pulses.append(pulse)

    return pulses


def generate_timed_charges(count, voltage):
    assert len(mode_list) == len(time_list)

    charges = []
    for i in range(count):
        event_info = EventInfo()
        event_info.id = i
        event_info.execution_condition.value = 2
        event_info.execution_time = 1.0 * i + 0.5 + WAIT

        charge = Charge()
        charge.channel = 1
        charge.target_voltage = voltage
        charge.event_info = event_info

        charges.append(charge)

    return charges



def generate_standard_charge_command(voltage):
    charges = []
    for i, _ in enumerate(time_list):
        event_info = EventInfo()
        event_info.id = i
        event_info.execution_condition.value = 2
        event_info.execution_time = 0.0

        charge = Charge()
        charge.channel = i + 1
        charge.target_voltage = voltage
        charge.event_info = event_info

        charges.append(charge)

    return charges
