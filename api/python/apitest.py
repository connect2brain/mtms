from event_interfaces.msg import ExecutionCondition, WaveformPhase

from MTMSApi import MTMSApi

api = MTMSApi()

api.start_device()
api.start_experiment()

waveform = api.get_default_waveform(channel=5)

api.send_pulse(waveform=waveform, channel=1, execution_condition=ExecutionCondition.IMMEDIATE, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=2, execution_condition=ExecutionCondition.IMMEDIATE, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=3, execution_condition=ExecutionCondition.IMMEDIATE, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=4, execution_condition=ExecutionCondition.IMMEDIATE, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=5, execution_condition=ExecutionCondition.IMMEDIATE, reverse_polarity=False)

api.wait(1)

api.end()
