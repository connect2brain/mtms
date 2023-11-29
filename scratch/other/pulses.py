import time as t

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


api = MTMSApi()

api.start_device()

api.stop_session()
api.start_session()

try:
    while True:
        print("")
        target_voltages = 5 * [None]
        for channel in range(5):
            target_voltages[channel] = int(input('Target voltage for channel {} (0-1499): '.format(channel + 1)))

        reverse_polarities = [voltage < 0 for voltage in target_voltages]
        target_voltages = [abs(voltage) for voltage in target_voltages]

        print("")

        for i in range(500):
            for n in range(2):
                start_time = t.time()

                execution_condition = ExecutionCondition.IMMEDIATE
                api.send_immediate_charge_or_discharge_to_all_channels(
                    target_voltages=target_voltages,
                )

#                a = input()

                mid_time = t.time()

                waveform = api.get_default_waveform(channel)
                reverse_polarity = False
                time = 0.0

                # Send trigger out on port 1.
                port = 1
                duration_us = 100
                api.send_trigger_out(
                    port=port,
                    duration_us=duration_us,
                    execution_condition=ExecutionCondition.WAIT_FOR_TRIGGER,
                    time=time,
                    wait_for_completion=False,
                )

                # Send pulse on the channel, using the default waveform.
                ids = api.send_default_pulse_to_all_channels(
                    reverse_polarities=reverse_polarities,
                    execution_condition=ExecutionCondition.WAIT_FOR_TRIGGER,
                    time=time,
                    wait_for_completion=False,
                )
                end_time = t.time()

                print("")
                print("Charging time: {:.3f}".format(mid_time - start_time))
                print("Whole time: {:.3f}".format(end_time - start_time))
                print("")

                api._wait_for_completions(ids)

except KeyboardInterrupt:
    api.stop_session()
