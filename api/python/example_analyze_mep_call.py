from event_interfaces.msg import ExecutionCondition, WaveformPhase
from mep_interfaces.msg import MepConfiguration, PreactivationCheck
from eeg_interfaces.msg import TimeWindow

from MTMSApi import MTMSApi

api = MTMSApi()

mep_configuration = MepConfiguration(
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
        voltage_range_limit=20.0,  # Maximum allowed voltage range inside the time window.
    ),
)

time = api.get_time() + 5.0

amplitude, latency, errors = api.analyze_mep(
    time=time,
    emg_channel=1,
    mep_configuration=mep_configuration,
)

print(amplitude, latency)

api.end()
