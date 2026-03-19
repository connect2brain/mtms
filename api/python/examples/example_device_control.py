from MTMSApi import MTMSApi

from event_interfaces.msg import (
    ExecutionCondition,
)
from mep_interfaces.srv import AnalyzeMep
from waveform_interfaces.msg import (
    WaveformPhase,
    WaveformPiece,
    Waveform
)
from targeting_interfaces.msg import ElectricTarget


api = MTMSApi()

api.start_device()
api.start_session()

## Single events

# Charge channel 0 to 20 V.

target_voltage = 20

# Note that the TMS channel indexing starts from 0.
channel = 1
execution_condition = ExecutionCondition.IMMEDIATE

api.send_charge(
    channel=channel,
    target_voltage=target_voltage,
    execution_condition=execution_condition,
)
api.wait_for_completion()

# Execute pulse on channel 0, using the default waveform.
channel = 2

waveform = api.get_default_waveform(channel)
reverse_polarity = False

execution_condition = ExecutionCondition.IMMEDIATE

api.send_pulse(
    channel=channel,
    waveform=waveform,
    execution_condition=execution_condition,
    reverse_polarity=reverse_polarity,
)
api.wait_for_completion()

# Discharge channel 0 completely.
target_voltage = 0

channel = 0
execution_condition = ExecutionCondition.IMMEDIATE

api.send_discharge(
    channel=channel,
    target_voltage=target_voltage,
    execution_condition=execution_condition,
)
api.wait_for_completion()

# Send trigger out on port 1.
port = 1
duration_us = 1000

execution_condition = ExecutionCondition.IMMEDIATE

api.send_trigger_out(
    port=port,
    duration_us=duration_us,
    execution_condition=execution_condition,
)
api.wait_for_completion()

# Send pulse on channel 0 and a simultaneous trigger out on port 1.
waveform = api.get_default_waveform(channel)
reverse_polarity = False

channel = 0
execution_condition = ExecutionCondition.TIMED
time = api.get_time() + 1.0

api.send_pulse(
    channel=channel,
    waveform=waveform,
    reverse_polarity=reverse_polarity,
    execution_condition=execution_condition,
    time=time,
)
# Do not wait for completion here, as we want to execute the trigger out simultaneously with the pulse.

port = 1
duration_us = 1000

# Use the same time and execution condition as for the pulse.
api.send_trigger_out(
    port=port,
    duration_us=duration_us,
    execution_condition=execution_condition,
    time=time,
)

# Once both pulse and trigger out are sent, wait for the completion of both.
api.wait_for_completion()

## Send pulse on channel 1 and analyze MEP.

# Use default waveform for the pulse.
waveform = api.get_default_waveform(channel)
reverse_polarity = False

# Generate a timed pulse.
channel = 1
execution_condition = ExecutionCondition.TIMED
time = api.get_time() + 3.0

api.send_pulse(
    channel=channel,
    waveform=waveform,
    execution_condition=execution_condition,
    time=time,
    reverse_polarity=reverse_polarity,
)
# Do not wait for completion here, as we want to execute the pulse simultaneously with the MEP analysis.

# MEP analysis is based on trigger coinciding with the pulse: thus, send a trigger out.
port = 1
duration_us = 1000

# Use the same time and execution condition as for the pulse.
api.send_trigger_out(
    port=port,
    duration_us=duration_us,
    execution_condition=execution_condition,
    time=time,
)

# Analyze MEP on the first EMG channel.
mep_configuration = {
    "emg_channel": 0,  # Indexing starts from 0
    "mep_time_window_start": 0.020,  # s, after stimulation pulse
    "mep_time_window_end": 0.040,  # s
    "preactivation_check_enabled": True,
    "preactivation_check_time_window_start": -0.040,  # s, before stimulation pulse
    "preactivation_check_time_window_end": -0.020,  # s
    "preactivation_check_voltage_range_limit": 70.0,  # uV
}

mep, status = api.analyze_mep(
    time=time,
    mep_configuration=mep_configuration,
)

if status == AnalyzeMep.Response.NO_ERROR:
    print(f"MEP analysis: amplitude={mep['amplitude']}, latency={mep['latency']}")
else:
    print(f"MEP analysis failed: status={status}")


## Targeting

# Define the target.

# Available algorithms:
#   ElectricTarget.LEAST_SQUARES
#   ElectricTarget.GENETIC

algorithm = ElectricTarget.GENETIC
displacement_x = 0  # mm
displacement_y = 0  # mm
rotation_angle = 45  # deg
intensity = 10  # V/m

target = ElectricTarget(
    displacement_x=displacement_x,
    displacement_y=displacement_y,
    rotation_angle=rotation_angle,
    intensity=intensity,
    algorithm=algorithm,
)

target_voltages, reverse_polarities = api.get_target_voltages(target)

# Charge all channels to target voltages.
api.send_immediate_charge_or_discharge_to_all_channels(
    target_voltages=target_voltages,
)
api.wait_for_completion()

# Send default pulse to all channels.
api.send_immediate_default_pulse_to_all_channels(
    reverse_polarities=reverse_polarities,
)
api.wait_for_completion()


## Paired pulse targeting

algorithm = ElectricTarget.GENETIC

# Define the targets.
first_target = ElectricTarget(
    displacement_x=0,  # mm
    displacement_y=0,  # mm
    rotation_angle=45,  # deg
    intensity=10,  # V/m
    algorithm=algorithm,
)

second_target = ElectricTarget(
    displacement_x=5,  # mm
    displacement_y=5,  # mm
    rotation_angle=90,  # deg
    intensity=5,  # V/m
    algorithm=algorithm,
)

targets = [first_target, second_target]

initial_voltages, approximated_waveforms = api.get_multipulse_waveforms(targets)

# Charge all channels to initial voltages.
api.send_immediate_charge_or_discharge_to_all_channels(
    target_voltages=initial_voltages,
)
api.wait_for_completion()

# Set the times (in seconds).
time = api.get_time() + 1.0
time_between_pulses = 0.003

# Send the first pulse.
api.send_timed_pulse_to_all_channels(
    waveforms_for_coil_set=approximated_waveforms[0],
    time=time,
)

# Send the second pulse.
api.send_timed_pulse_to_all_channels(
    waveforms_for_coil_set=approximated_waveforms[1],
    time=time + time_between_pulses,
)

# Wait for completion of both pulses.
api.wait_for_completion()


## Targeting-related utilities

# Get maximum intensity
algorithm = ElectricTarget.GENETIC
displacement_x = 0  # mm
displacement_y = 0  # mm
rotation_angle = 45  # deg

maximum_intensity = api.get_maximum_intensity(
    displacement_x=displacement_x,
    displacement_y=displacement_y,
    rotation_angle=rotation_angle,
    algorithm=algorithm,
)


## Targeting combined with MEP analysis

displacement_x = 5  # mm
displacement_y = 5  # mm
rotation_angle = 90  # deg
intensity = 5  # V/m

target_voltages, reverse_polarities = api.get_target_voltages(
    displacement_x=displacement_x,
    displacement_y=displacement_y,
    rotation_angle=rotation_angle,
    intensity=intensity,
    algorithm=ElectricTarget.GENETIC
)

# Charge all channels to target voltages.
api.send_immediate_charge_or_discharge_to_all_channels(
    target_voltages=target_voltages,
)
api.wait_for_completion()

# Send default pulse to all channels.
time = api.get_time() + 3.0

api.send_timed_default_pulse_to_all_channels(
    reverse_polarities=reverse_polarities,
    time=time,
)
# Do not wait for completion here, as we want to execute the pulse simultaneously with the MEP analysis.

# MEP analysis is based on trigger coinciding with the pulse: thus, send a trigger out.
port = 1
duration_us = 1000

execution_condition = ExecutionCondition.TIMED

api.send_trigger_out(
    port=port,
    duration_us=duration_us,
    execution_condition=execution_condition,
    time=time,
)

# Analyze MEP on the first EMG channel.
mep_configuration = {
    "emg_channel": 0,  # Indexing starts from 0
    "mep_time_window_start": 0.020,  # s, after stimulation pulse
    "mep_time_window_end": 0.040,  # s
    "preactivation_check_enabled": True,
    "preactivation_check_time_window_start": -0.040,  # s, before stimulation pulse
    "preactivation_check_time_window_end": -0.020,  # s
    "preactivation_check_voltage_range_limit": 70.0,  # uV
}

mep, status = api.analyze_mep(
    time=time,
    mep_configuration=mep_configuration,
)

if status == AnalyzeMep.Response.NO_ERROR:
    print(f"MEP analysis: amplitude={mep['amplitude']}, latency={mep['latency']}")
else:
    print(f"MEP analysis failed: status={status}")

## Define custom waveform

waveform = Waveform(
    pieces=[
        WaveformPiece(
            waveform_phase=WaveformPhase(
                value=WaveformPhase.RISING
            ),
            duration_in_ticks=2400,
        ),
        WaveformPiece(
            waveform_phase=WaveformPhase(
                value=WaveformPhase.HOLD
            ),
            duration_in_ticks=1200,
        ),
        WaveformPiece(
            waveform_phase=WaveformPhase(
                value=WaveformPhase.FALLING
            ),
            duration_in_ticks=1480,
        ),
    ]
)

## Restart session
api.stop_session()
api.start_session()


## Stop device
api.stop_device()
