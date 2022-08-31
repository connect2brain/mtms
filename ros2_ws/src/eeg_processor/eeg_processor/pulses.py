from fpga_interfaces.msg import EventInfo, StimulationPulsePiece, StimulationPulseEvent, ChargeEvent

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


def generate_standard_pulse_command():
    assert len(mode_list) == len(time_list)

    events = []
    for i, _ in enumerate(time_list):
        pieces = []
        for j, _ in enumerate(time_list[i]):
            piece = StimulationPulsePiece()
            piece.mode = mode_list[i][j]
            piece.duration_in_ticks = int(time_list[i][j] * US_TO_TICKS_CONVERSION_RATIO)
            pieces.append(piece)

        event_info = EventInfo()
        event_info.event_id = i
        event_info.execution_condition = 2
        event_info.time_us = 0

        event = StimulationPulseEvent()
        event.event_info = event_info
        event.channel = i + 1
        event.pieces = pieces

        events.append(event)

    return events

WAIT = 10

def generate_timed_pulses(count):
    assert len(mode_list) == len(time_list)

    events = []
    for i in range(count):
        pieces = []
        for j, _ in enumerate(time_list[0]):
            piece = StimulationPulsePiece()
            piece.mode = mode_list[0][j]
            piece.duration_in_ticks = int(time_list[0][j] * US_TO_TICKS_CONVERSION_RATIO)
            pieces.append(piece)

        event_info = EventInfo()
        event_info.event_id = i
        event_info.execution_condition = 0
        event_info.time_us = int(1e6 * i + WAIT)

        event = StimulationPulseEvent()
        event.event_info = event_info
        event.channel = 1
        event.pieces = pieces

        events.append(event)

    return events


def generate_timed_charges(count, voltage):
    assert len(mode_list) == len(time_list)

    events = []
    for i in range(count):
        event_info = EventInfo()
        event_info.event_id = i
        event_info.execution_condition = 2
        event_info.time_us = int(1e6 * i + 1e6/2 + WAIT)

        event = ChargeEvent()
        event.channel = 1
        event.target_voltage = voltage
        event.event_info = event_info
        events.append(event)

    return events



def generate_standard_charge_command(voltage):
    events = []
    for i, _ in enumerate(time_list):
        event_info = EventInfo()
        event_info.event_id = i
        event_info.execution_condition = 2
        event_info.time_us = 0

        event = ChargeEvent()
        event.channel = i + 1
        event.target_voltage = voltage
        event.event_info = event_info

        events.append(event)

    return events
