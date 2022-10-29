from fpga_interfaces.msg import ExecutionCondition, CurrentMode

from mtms_api.MTMSApi import MTMSApi

api = MTMSApi()

api.start_device()
api.start_experiment()

waveform = api.get_default_waveform(channel=5)

api.send_pulse(waveform=waveform, channel=1, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=2, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=3, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=4, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
api.send_pulse(waveform=waveform, channel=5, id=1, execution_condition=ExecutionCondition.INSTANT, reverse_polarity=False)
#api.send_charge(id=2, channel=5, execution_condition=ExecutionCondition.INSTANT, target_voltage=10)
#api.send_discharge(id=3, channel=5, execution_condition=ExecutionCondition.INSTANT, target_voltage=0)
#api.send_signal_out(id=4, port=1, execution_condition=ExecutionCondition.INSTANT, duration_us=1000)
#api.wait_forever()

#voltages, reverse_polarities = api.get_channel_voltages(
#    displacement_x=5,
#    displacement_y=4,
#    rotation_angle=45,
#    intensity=5,
#)

#api.charge_all_channels_instantly(
#    target_voltages=voltages,
#)

# api.send_pulse_to_all_channels(
#     waveform=waveform,
#     reverse_polarities=reverse_polarities,
#     execution_condition=ExecutionCondition.TRIGGERED,
#     starting_id=1
# )

#api.send_event_trigger()
api.wait(2)

#api.wait_forever()
#api.wait(1)
#api.discharge_all_channels_instantly(
#    [0, 0, 0, 0, 0]
#)

#api.stop_device()

api.end()
