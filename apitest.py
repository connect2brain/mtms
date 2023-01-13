from mtms_api.enums.ExecutionCondition import ExecutionCondition
from mtms_api.enums.PulseMode import PulseMode

from mtms_api.MTMSApi import MTMSApi

api = MTMSApi()

api.start_device()
api.start_experiment()

waveform = api.get_default_waveform(channel=5)

api.send_pulse(waveform=waveform, channel=5, id=1, execution_condition=ExecutionCondition.INSTANT)
#api.send_charge(id=2, channel=5, execution_condition=ExecutionCondition.INSTANT, target_voltage=10)
api.send_discharge(id=3, channel=5, execution_condition=ExecutionCondition.INSTANT, target_voltage=0)
api.send_signal_out(id=4, port=1, execution_condition=ExecutionCondition.INSTANT, duration_us=1000)

api.wait(5)

api.stop_device()

api.end()
