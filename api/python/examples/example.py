from MTMSApi import MTMSApi

from event_interfaces.msg import (
    ExecutionCondition,
    WaveformPhase
)
from mep_interfaces.msg import (
    MepConfiguration,
    PreactivationCheck
)
from eeg_interfaces.msg import TimeWindow
from targeting_interfaces.msg import TargetingAlgorithm


api = MTMSApi()

api.start_device()
api.start_session()


## Single events

# Charge channel 1 to 20 V.

channel = 0  # Note that channel indexing starts at 0.
target_voltage = 20
execution_condition = ExecutionCondition.IMMEDIATE
time = 10.0

api.send_charge(
    channel=channel,
    target_voltage=target_voltage,
    execution_condition=execution_condition,
    time=time,
)

# Send pulse on channel 1, using the default waveform.
waveform = api.get_default_waveform(channel)
reverse_polarity = False

api.allow_stimulation(True)

api.send_pulse(
    channel=channel,
    waveform=waveform,
    execution_condition=execution_condition,
    time=time,
    reverse_polarity=reverse_polarity,
)

# Discharge channel 1 completely.
api.send_discharge(
    channel=channel,
    target_voltage=0,
    execution_condition=execution_condition,
    time=time,
)

# Send trigger out on port 1.
port = 1
duration_us = 1000

api.send_trigger_out(
    port=port,
    duration_us=duration_us,
    execution_condition=execution_condition,
    time=time,
)

# Send pulse on channel 1 and a simultaneous trigger out on port 1.
channel = 1
waveform = api.get_default_waveform(channel)
reverse_polarity = False
execution_condition = ExecutionCondition.TIMED
time = api.get_time() + 1.0

api.allow_stimulation(True)

api.send_pulse(
    channel=channel,
    waveform=waveform,
    execution_condition=execution_condition,
    time=time,
    reverse_polarity=reverse_polarity,
    wait_for_completion=False,
)

port = 1
duration_us = 1000

api.send_trigger_out(
    port=port,
    duration_us=duration_us,
    execution_condition=execution_condition,
    time=time,
    wait_for_completion=False,
)

api.wait(2)

## Send pulse on channel 1 and analyze MEP.

# Use default waveform for the pulse.
waveform = api.get_default_waveform(channel)
reverse_polarity = False

# Generate a timed pulse.
channel = 1
execution_condition = ExecutionCondition.TIMED
time = api.get_time() + 3.0
wait_for_completion = False  # Note that this needs to be false so that MEP can be queried for before the pulse is executed.

api.send_pulse(
    channel=channel,
    waveform=waveform,
    execution_condition=execution_condition,
    time=time,
    reverse_polarity=reverse_polarity,
    wait_for_completion=wait_for_completion,
)

# Analyze MEP on EMG channel 1, coinciding with the pulse.
mep_configuration = MepConfiguration(
    emg_channel=1,

    time_window=TimeWindow(
        start=0.020,  # in ms, after the stimulation pulse
        end=0.040,  # in ms
    ),
    preactivation_check=PreactivationCheck(
        enabled=True,
        time_window=TimeWindow(
            start=-0.040,  # in ms, minus sign indicates that the window starts before the stimulation pulse
            end=-0.020,
        ),
        voltage_range_limit=70.0,  # Maximum allowed voltage range inside the time window, in uV.
    ),
)

mep, errors = api.analyze_mep(
    time=time,
    mep_configuration=mep_configuration,
)

amplitude = mep.amplitude
latency = mep.latency


## Targeting

displacement_x = 5  # mm
displacement_y = 5  # mm
rotation_angle = 90  # deg
intensity = 20  # V/m

target_voltages, reverse_polarities = api.get_channel_voltages(
    displacement_x=displacement_x,
    displacement_y=displacement_y,
    rotation_angle=rotation_angle,
    intensity=intensity,
    algorithm=TargetingAlgorithm.GENETIC,
)

# Get maximum intensity
maximum_intensity = api.get_maximum_intensity(
    displacement_x=displacement_x,
    displacement_y=displacement_y,
    rotation_angle=rotation_angle,
    algorithm=TargetingAlgorithm.GENETIC
)

# Charge all channels to target voltages.
api.send_immediate_charge_or_discharge_to_all_channels(
    target_voltages=target_voltages,
)

# Send default pulse to all channels.
api.send_immediate_default_pulse_to_all_channels(
    reverse_polarities=reverse_polarities,
)


## Targeting combined with MEP analysis

displacement_x = 5  # mm
displacement_y = 5  # mm
rotation_angle = 90  # deg
intensity = 20  # V/m

target_voltages, reverse_polarities = api.get_channel_voltages(
    displacement_x=displacement_x,
    displacement_y=displacement_y,
    rotation_angle=rotation_angle,
    intensity=intensity,
)

# Charge all channels to target voltages.
api.send_immediate_charge_or_discharge_to_all_channels(
    target_voltages=target_voltages,
)

# Send default pulse to all channels.
wait_for_completion = False  # Note that this needs to be false so that MEP can be queried for before the pulse is executed.
time = api.get_time() + 3.0

api.send_timed_default_pulse_to_all_channels(
    reverse_polarities=reverse_polarities,
    time=time,
    wait_for_completion=wait_for_completion,
)

# Analyze MEP on EMG channel 1, coinciding with the pulse.
mep_configuration = MepConfiguration(
    emg_channel=1,

    time_window=TimeWindow(
        start=0.020,  # in ms, after the stimulation pulse
        end=0.040,  # in ms
    ),
    preactivation_check=PreactivationCheck(
        enabled=True,
        time_window=TimeWindow(
            start=-0.040,  # in ms, minus sign indicates that the window starts before the stimulation pulse
            end=-0.020,
        ),
        voltage_range_limit=70.0,  # Maximum allowed voltage range inside the time window, in uV.
    ),
)

mep, errors = api.analyze_mep(
    time=time,
    mep_configuration=mep_configuration,
)

amplitude = mep.amplitude
latency = mep.latency

## Restart session
api.stop_session()
api.start_session()


## Stop device
api.stop_device()
