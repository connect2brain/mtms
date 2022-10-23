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

    # Events
    def send_pulse(self, id, channel, waveform, execution_condition=ExecutionCondition.TIMED, time=0, reverse_polarity=False):
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

    def send_charge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        target_voltage = int(target_voltage)
        self.node.send_charge(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            target_voltage=target_voltage,
        )

    def send_discharge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        target_voltage = int(target_voltage)
        self.node.send_discharge(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            target_voltage=target_voltage,
        )

    def send_signal_out(self, id, port, duration_us, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        self.node.send_signal_out(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            port=port,
            duration_us=duration_us,
        )

    def send_event_trigger(self):
        self.node.send_event_trigger()

        # HACK: This should probably be a feedback message that is received from the FPGA, informing that the
        #   event trigger was successfully generated, similar to the feedback messages from the actual events.
        #
        print('{}Event trigger{}'.format(bcolors.OKGREEN, bcolors.ENDC))

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

    def charge_all_channels_instantly(self, target_voltages, starting_id=1):
        assert len(target_voltages) == self.N_CHANNELS, "Target voltage only defined for {} channels, channel count: {}.".format(
            len(target_voltages), self.N_CHANNELS)

        for i in range(self.N_CHANNELS):
            target_voltage = target_voltages[i]
            channel = i + 1
            id = starting_id + i

            self.send_charge(
                id=id,
                execution_condition=ExecutionCondition.INSTANT,
                time=0,
                channel=channel,
                target_voltage=target_voltage,
            )

    def discharge_all_channels_instantly(self, target_voltages, starting_id=1):
        assert len(target_voltages) == self.N_CHANNELS, "Target voltage only defined for {} channels, channel count: {}.".format(
            len(target_voltages), self.N_CHANNELS)

        for i in range(self.N_CHANNELS):
            target_voltage = target_voltages[i]
            channel = i + 1
            id = starting_id + i

            self.send_discharge(
                id=id,
                execution_condition=ExecutionCondition.INSTANT,
                time=0,
                channel=channel,
                target_voltage=target_voltage,
            )

    def send_pulse_to_all_channels(self, waveform, reverse_polarities, execution_condition=ExecutionCondition.TIMED, time=0, starting_id=1):
        assert len(reverse_polarities) == self.N_CHANNELS, "Reverse polarities only defined for {} channels, channel count: {}.".format(
            len(reverse_polarities), self.N_CHANNELS)

        for i in range(self.N_CHANNELS):
            reverse_polarity = reverse_polarities[i]
            channel = i + 1
            id = starting_id + i

            self.send_pulse(
                id=id,
                execution_condition=execution_condition,
                time=time,
                channel=channel,
                waveform=waveform,
                reverse_polarity=reverse_polarity,
            )

    # Other

    def print_system_state(self):
        self.node.wait_for_new_state()

    def get_wallclock_time(self):
        return time.time()

    def end(self):
        self.node.destroy_node()
        rclpy.shutdown()
