import time

import rclpy

from .enums.DeviceState import DeviceState
from .enums.ExecutionCondition import ExecutionCondition
from .enums.ExperimentState import ExperimentState
from .enums.PulseMode import PulseMode

from .MTMSApiNode import MTMSApiNode

class MTMSApi:
    def __init__(self):
        rclpy.init(args=None)
        self.bridge = MTMSApiNode()

    # Start and stop

    def start_device(self):
        self.bridge.start_device()
        while self.get_device_state() != DeviceState.OPERATIONAL:
            pass

    def stop_device(self):
        self.bridge.stop_device()
        while self.get_device_state() != DeviceState.NOT_OPERATIONAL:
            pass

    def start_experiment(self):
        self.bridge.start_experiment()
        while self.get_experiment_state() != ExperimentState.STARTED:
            pass

    def stop_experiment(self):
        self.bridge.stop_experiment()
        while self.get_experiment_state() != ExperimentState.STOPPED:
            pass

    # Wait

    def wait_forever(self):
        while True:
            self.bridge.wait_for_new_state()

    def wait_until(self, time):
        self.bridge.wait_for_new_state()
        while self.get_time() < time:
            self.bridge.wait_for_new_state()

    def wait(self, time):
        start_time = self.get_wallclock_time()

        self.bridge.wait_for_new_state()
        while self.get_wallclock_time() < start_time + time:
            self.bridge.wait_for_new_state()

    # Getters

    def get_device_state(self):
        self.bridge.wait_for_new_state()
        return DeviceState(self.bridge.system_state.device_state)

    def get_experiment_state(self):
        self.bridge.wait_for_new_state()
        return ExperimentState(self.bridge.system_state.experiment_state)

    def get_voltage(self, channel):
        self.bridge.wait_for_new_state()
        return self.bridge.system_state.channel_states[channel - 1].voltage

    def get_temperature(self, channel):
        self.bridge.wait_for_new_state()
        return self.bridge.system_state.channel_states[channel - 1].temperature

    def get_pulse_count(self, channel):
        self.bridge.wait_for_new_state()
        return self.bridge.system_state.channel_states[channel - 1].pulse_count

    def get_time(self):
        self.bridge.wait_for_new_state()
        return self.bridge.system_state.time

    # Events
    def send_pulse(self, id, channel, waveform, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        self.bridge.send_pulse(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            waveform=waveform,
        )

    def send_charge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        self.bridge.send_charge(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            target_voltage=target_voltage,
        )

    def send_discharge(self, id, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        self.bridge.send_discharge(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            channel=channel,
            target_voltage=target_voltage,
        )

    def send_signal_out(self, id, port, duration_us, execution_condition=ExecutionCondition.TIMED, time=0):
        time_us = int(time * 10**6)
        self.bridge.send_signal_out(
            id=id,
            execution_condition=execution_condition,
            time_us=time_us,
            port=port,
            duration_us=duration_us,
        )

    def send_event_trigger(self):
        self.bridge.send_event_trigger()

        # HACK: This should probably be a feedback message that is received from the FPGA, informing that the
        #   event trigger was successfully generated, similar to the feedback messages from the actual events.
        #
        print('{}Event trigger{}'.format(bcolors.OKGREEN, bcolors.ENDC))

    # Testing and debugging (undocumented)

    def start_device_without_waiting(self):
        self.bridge.start_device()

    def stop_device_without_waiting(self):
        self.bridge.stop_device()

    def start_experiment_without_waiting(self):
        self.bridge.start_experiment()

    def stop_experiment_without_waiting(self):
        self.bridge.stop_experiment()

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

    # Targeting
    def get_channel_voltages(self, displacement_x, displacement_y, rotation_angle, intensity):
        return self.bridge.get_channel_voltages(
            displacement_x=displacement_x,
            displacement_y=displacement_y,
            rotation_angle=rotation_angle,
            intensity=intensity,
        )

    # Other

    def print_system_state(self):
        self.bridge.wait_for_new_state()

    def get_wallclock_time(self):
        return time.time()

    def end(self):
        self.bridge.destroy_node()
        rclpy.shutdown()
