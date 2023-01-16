from event_interfaces.msg import ExecutionCondition, WaveformPhase

from MTMSApi import MTMSApi

api = MTMSApi()

api.start_device()
api.start_experiment()

waveform = api.get_default_waveform(channel=5)

api.send_pulse(waveform=waveform, channel=1, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=2, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=3, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=4, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=5, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)

api.wait(1)

api.end()
