from MTMSApi import MTMSApi

from event_interfaces.msg import (
    ExecutionCondition,
)

api = MTMSApi()

api.start_device()
api.start_session()

# Charge channel 2 to 800 V

# 800 V fails roughly half of the time with "triggering failure" on Channel 1 (!)

# Increasing voltage to 850 V makes it fail every time.

channel = 2
target_voltage = 850
execution_condition = ExecutionCondition.IMMEDIATE
time = 10.0
api.send_charge(
    channel=channel,
    target_voltage=target_voltage,
    execution_condition=execution_condition,
    time=time,
)

# Execute a pulse on all channels; channels 1 and 2 with reverse polarity

wait_for_completion = False  # Note that this needs to be false so that MEP can be queried for before the pulse is executed.
time = api.get_time() + 1.0

api.send_timed_default_pulse_to_all_channels(
    reverse_polarities=[False, True, True, False, False],
    time=time,
    wait_for_completion=wait_for_completion,
)
api.wait(2)
