import time
import copy

import rclpy

from .enums.DeviceState import DeviceState
from .enums.ExecutionCondition import ExecutionCondition
from .enums.ExperimentState import ExperimentState
from .enums.PulseMode import PulseMode

from .MTMSApiNode import MTMSApiNode

class MTMSApi:
    # TODO: Channel count hardcoded for now.
    N_CHANNELS = 5

    # TIME_EPSILON is used to implement events that are to be executed instantly but
    # wanting to synchronize them: to do that, get current time, add TIME_EPSILON to it,
    # and execute all the events at that time.
    #
    # Consequently, TIME_EPSILON must be large enough to allow time to send the events to
    # the mTMS device, but not too large so that the events are not executed 'instantly'.
    # Settle for 0.1 s (100 ms) for now, but change if needed.
    #
    TIME_EPSILON = 0.1

    def __init__(self):
        rclpy.init(args=None)
        self.node = MTMSApiNode()

    # Start and stop

    def start_device(self):
        self.node.start_device()
        while self.get_device_state() != DeviceState.OPERATIONAL:
            pass

    def stop_device(self):
        self.node.stop_device()
        while self.get_device_state() != DeviceState.NOT_OPERATIONAL:
            pass

    def start_experiment(self):
        self.node.start_experiment()
        while self.get_experiment_state() != ExperimentState.STARTED:
            pass

    def stop_experiment(self):
        self.node.stop_experiment()
        while self.get_experiment_state() != ExperimentState.STOPPED:
            pass

    # Wait

    def wait_forever(self):
        while True:
            self.node.wait_for_new_state()

    def wait_until(self, time):
        self.node.wait_for_new_state()
        while self.get_time() < time:
            self.node.wait_for_new_state()

    def wait_for_feedback(self, id):
        self.node.wait_for_new_state()
        while self.node.get_event_feedback(id) is None:
            self.node.wait_for_new_state()

    def wait_for_feedbacks(self, ids):
        for id in ids:
            self.wait_for_feedback(id)

    def wait(self, time):
        start_time = self.get_wallclock_time()

        self.node.wait_for_new_state()
        while self.get_wallclock_time() < start_time + time:
            self.node.wait_for_new_state()

    # Getters

    def get_device_state(self):
        self.node.wait_for_new_state()
        return DeviceState(self.node.system_state.device_state)

    def get_experiment_state(self):
        self.node.wait_for_new_state()
        return ExperimentState(self.node.system_state.experiment_state)

    def get_voltage(self, channel):
        self.node.wait_for_new_state()
        return self.node.system_state.channel_states[channel - 1].voltage

    def get_temperature(self, channel):
        self.node.wait_for_new_state()
        return self.node.system_state.channel_states[channel - 1].temperature


    def get_pulse_count(self, channel):
        self.node.wait_for_new_state()
        return self.node.system_state.channel_states[channel - 1].pulse_count

    def get_time(self):
        self.node.wait_for_new_state()
        return self.node.system_state.time

    def get_event_feedback(self, id):
        return self.node.get_event_feedback(id)

    # Events
    def send_pulse(self, id, channel, waveform, execution_condition=ExecutionCondition.TIMED, time=0, reverse_polarity=False, wait_for_feedback=True):
        time_us = int(time * 10**6)

        waveform_ = copy.deepcopy(waveform)
        if reverse_polarity:
            waveform_ = self.reverse_polarity(waveform_)

        self.node.send_pulse(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            waveform=waveform_,
        )

        if wait_for_feedback:
            self.wait_for_feedback(id=id)

    def send_charge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_feedback=True):
        time_us = int(time * 10**6)
        target_voltage = int(target_voltage)
        self.node.send_charge(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            target_voltage=target_voltage,
        )

        if wait_for_feedback:
            self.wait_for_feedback(id=id)

    def send_discharge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_feedback=True):
        time_us = int(time * 10**6)
        target_voltage = int(target_voltage)
        self.node.send_discharge(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            target_voltage=target_voltage,
        )

        if wait_for_feedback:
            self.wait_for_feedback(id=id)

    def send_signal_out(self, id, port, duration_us, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_feedback=True):
        time_us = int(time * 10**6)
        self.node.send_signal_out(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            port=port,
            duration_us=duration_us,
        )

        if wait_for_feedback:
            self.wait_for_feedback(id=id)

    def send_event_trigger(self):
        self.node.send_event_trigger()

    # Testing and debugging (undocumented)

    def start_device_without_waiting(self):
        self.node.start_device()

    def stop_device_without_waiting(self):
        self.node.stop_device()

    def start_experiment_without_waiting(self):
        self.node.start_experiment()

    def stop_experiment_without_waiting(self):
        self.node.stop_experiment()

    def wait_until_not_operational(self):
        while self.get_device_state() != DeviceState.NOT_OPERATIONAL:
            pass

    def wait_until_operational(self):
        while self.get_device_state() != DeviceState.OPERATIONAL:
            pass

    # Helpers

    def get_default_waveform(self, channel):
        assert 1<= channel <= 5

        if channel == 1 or channel == 2:
            falling_phase_duration_in_ticks = 1480
        elif channel == 3 or channel == 4:
            falling_phase_duration_in_ticks = 1564
        elif channel == 5:
            falling_phase_duration_in_ticks = 1776

        waveform = [
            {
                'mode': PulseMode.RISING,
                'duration_in_ticks': 2400
            },
            {
                'mode': PulseMode.HOLD,
                'duration_in_ticks': 1200
            },
            {
                'mode': PulseMode.FALLING,
                'duration_in_ticks': falling_phase_duration_in_ticks,
            },
        ]
        return waveform

    def reverse_polarity(self, waveform):
        for i in range(len(waveform)):
            mode = waveform[i]['mode']
            if mode == PulseMode.RISING:
                reversed_mode = PulseMode.FALLING
            elif mode == PulseMode.FALLING:
                reversed_mode = PulseMode.RISING
            elif mode == PulseMode.HOLD:
                reversed_mode = PulseMode.ALTERNATIVE_HOLD
            elif mode == PulseMode.ALTERNATIVE_HOLD:
                reversed_mode = PulseMode.HOLD

            waveform[i]['mode'] = reversed_mode

        return waveform

    # Targeting

    def get_channel_voltages(self, displacement_x, displacement_y, rotation_angle, intensity):
        return self.node.get_channel_voltages(
            displacement_x=displacement_x,
            displacement_y=displacement_y,
            rotation_angle=rotation_angle,
            intensity=intensity,
        )

    # Compound events

    def send_instant_charge_or_discharge_to_all_channels(self, target_voltages, starting_id=1, wait_for_feedback=True):
        assert len(target_voltages) == self.N_CHANNELS, "Target voltage only defined for {} channels, channel count: {}.".format(
            len(target_voltages), self.N_CHANNELS)

        ids = range(starting_id, starting_id + self.N_CHANNELS)

        for i in range(self.N_CHANNELS):
            id = ids[i]
            target_voltage = target_voltages[i]
            channel = i + 1

            self.send_charge_or_discharge(
                id=id,
                execution_condition=ExecutionCondition.INSTANT,
                time=0,
                channel=channel,
                target_voltage=target_voltage,
                wait_for_feedback=False,
            )

        if wait_for_feedback:
            self.wait_for_feedbacks(ids=ids)

    def send_instant_full_discharge_to_all_channels(self, starting_id=1, wait_for_feedback=True):
        target_voltages = self.N_CHANNELS * [0]

        self.send_instant_charge_or_discharge_to_all_channels(
            target_voltages=target_voltages,
            starting_id=starting_id,
            wait_for_feedback=wait_for_feedback,
        )

    def send_default_pulse_to_all_channels(self, reverse_polarities, execution_condition=ExecutionCondition.TIMED, time=0, starting_id=1, wait_for_feedback=True):
        assert len(reverse_polarities) == self.N_CHANNELS, "Reverse polarities only defined for {} channels, channel count: {}.".format(
            len(reverse_polarities), self.N_CHANNELS)

        ids = range(starting_id, starting_id + self.N_CHANNELS)

        for i in range(self.N_CHANNELS):
            id = ids[i]
            reverse_polarity = reverse_polarities[i]
            channel = i + 1
            waveform = self.get_default_waveform(channel=channel)

            self.send_pulse(
                id=id,
                execution_condition=execution_condition,
                time=time,
                channel=channel,
                waveform=waveform,
                reverse_polarity=reverse_polarity,
                wait_for_feedback=False,
            )

        if wait_for_feedback:
            self.wait_for_feedbacks(ids=ids)

    def send_instant_default_pulse_to_all_channels(self, reverse_polarities, starting_id=1, wait_for_feedback=True):
        execution_condition = ExecutionCondition.TIMED
        time = self.get_time() + self.TIME_EPSILON

        self.send_default_pulse_to_all_channels(
            reverse_polarities=reverse_polarities,
            execution_condition=execution_condition,
            time=time,
            starting_id=starting_id,
            wait_for_feedback=wait_for_feedback,
        )

    def send_charge_or_discharge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_feedback=True):
        voltage = self.get_voltage(channel=channel)
        charge_or_discharge = self.send_charge if voltage < target_voltage else self.send_discharge
        charge_or_discharge(
            id=id,
            channel=channel,
            target_voltage=target_voltage,
            execution_condition=execution_condition,
            time=time,
            wait_for_feedback=wait_for_feedback,
        )

    # Other

    def print_system_state(self):
        self.node.wait_for_new_state()

    def get_wallclock_time(self):
        return time.time()

    def end(self):
        self.node.destroy_node()
        rclpy.shutdown()
