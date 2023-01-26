from event_interfaces.msg import ExecutionCondition, WaveformPhase

from MTMSApi import MTMSApi

api = MTMSApi()

api.start_device()
api.start_experiment()

api.send_charge(
    id=1,
    execution_condition=ExecutionCondition.TIMED,
    time=1.0,
    channel=1,
    target_voltage=10,
)

waveform = api.get_default_waveform(channel=1)

api.send_pulse(
    id=2,
    execution_condition=ExecutionCondition.TIMED,
    time=2.0,
    channel=1,
    waveform=waveform,
)

api.wait_until(2.5)
api.stop_device()

api.end()
