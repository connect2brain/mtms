import time
import copy

import rclpy

from mtms_device_interfaces.msg import ExperimentState, DeviceState
from event_interfaces.msg import ExecutionCondition, WaveformPhase

from MTMSApiNode import MTMSApiNode

class MTMSApi:
    # TODO: Channel count hardcoded for now.
    N_CHANNELS = 5

    # TIME_EPSILON is used to implement events that are to be executed immediately but
    # wanting to synchronize them: to do that, get current time, add TIME_EPSILON to it,
    # and execute all the events at that time.
    #
    # Consequently, TIME_EPSILON must be large enough to allow time to send the events to
    # the mTMS device, but not too large so that the events are not executed 'immediately'.
    # Settle for 0.1 s (100 ms) for now, but change if needed.
    #
    TIME_EPSILON = 0.1

    def __init__(self):
        rclpy.init(args=None)
        self.node = MTMSApiNode()
        self.event_id = 0

    # General

    def _next_event_id(self):
        self.event_id += 1
        return self.event_id

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

    def _wait_for_completion(self, id):
        self.node.wait_for_new_state()
        while self.node.get_event_feedback(id) is None:
            self.node.wait_for_new_state()

    def _wait_for_completions(self, ids):
        for id in ids:
            self._wait_for_completion(id)

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
        return self.node.system_state.device_state.value

    def get_experiment_state(self):
        self.node.wait_for_new_state()
        return self.node.system_state.experiment_state.value

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
    def send_pulse(self, channel, waveform, execution_condition=ExecutionCondition.TIMED, time=0.0, reverse_polarity=False, wait_for_completion=True):
        id = self._next_event_id()

        waveform_ = copy.deepcopy(waveform)
        if reverse_polarity:
            waveform_ = self.reverse_polarity(waveform_)

        self.node.send_pulse(
            id=id,
            execution_condition=execution_condition,
            time=time,
            channel=channel,
            waveform=waveform_,
        )

        if wait_for_completion:
            self._wait_for_completion(id=id)

        return id

    def send_charge(self, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_completion=True):
        id = self._next_event_id()

        target_voltage = int(target_voltage)
        self.node.send_charge(
            id=id,
            execution_condition=execution_condition,
            time=time,
            channel=channel,
            target_voltage=target_voltage,
        )

        if wait_for_completion:
            self._wait_for_completion(id=id)

        return id

    def send_discharge(self, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_completion=True):
        id = self._next_event_id()

        target_voltage = int(target_voltage)
        self.node.send_discharge(
            id=id,
            execution_condition=execution_condition,
            time=time,
            channel=channel,
            target_voltage=target_voltage,
        )

        if wait_for_completion:
            self._wait_for_completion(id=id)

        return id

    def send_trigger_out(self, port, duration_us, execution_condition=ExecutionCondition.TIMED, time=0, wait_for_completion=True):
        id = self._next_event_id()

        self.node.send_trigger_out(
            id=id,
            execution_condition=execution_condition,
            time=time,
            port=port,
            duration_us=duration_us,
        )

        if wait_for_completion:
            self._wait_for_completion(id=id)

        return id

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
        return self.node.get_default_waveform(channel=channel)

    def reverse_polarity(self, waveform):
        return self.node.reverse_polarity(waveform=waveform)

    # Targeting

    def get_channel_voltages(self, displacement_x, displacement_y, rotation_angle, intensity):
        return self.node.get_channel_voltages(
            displacement_x=displacement_x,
            displacement_y=displacement_y,
            rotation_angle=rotation_angle,
            intensity=intensity,
        )

    def get_maximum_intensity(self, displacement_x, displacement_y, rotation_angle):
        return self.node.get_maximum_intensity(
            displacement_x=displacement_x,
            displacement_y=displacement_y,
            rotation_angle=rotation_angle,
        )

    # Compound events

    def send_immediate_charge_or_discharge_to_all_channels(self, target_voltages, wait_for_completion=True):
        assert len(target_voltages) == self.N_CHANNELS, "Target voltage only defined for {} channels, channel count: {}.".format(
            len(target_voltages), self.N_CHANNELS)

        ids = []
        for i in range(self.N_CHANNELS):
            target_voltage = target_voltages[i]
            channel = i + 1

            id = self.send_charge_or_discharge(
                execution_condition=ExecutionCondition.IMMEDIATE,
                channel=channel,
                target_voltage=target_voltage,
                wait_for_completion=False,
            )
            ids.append(id)

        if wait_for_completion:
            self._wait_for_completions(ids=ids)

        return ids

    def send_immediate_full_discharge_to_all_channels(self, wait_for_completion=True):
        target_voltages = self.N_CHANNELS * [0]

        ids = self.send_immediate_charge_or_discharge_to_all_channels(
            target_voltages=target_voltages,
            wait_for_completion=wait_for_completion,
        )

        return ids

    def send_timed_default_pulse_to_all_channels(self, reverse_polarities, time=0.0, wait_for_completion=True):
        assert len(reverse_polarities) == self.N_CHANNELS, "Reverse polarities only defined for {} channels, channel count: {}.".format(
            len(reverse_polarities), self.N_CHANNELS)

        ids = []
        for i in range(self.N_CHANNELS):
            reverse_polarity = reverse_polarities[i]
            channel = i + 1
            waveform = self.get_default_waveform(channel=channel)

            id = self.send_pulse(
                execution_condition=ExecutionCondition.TIMED,
                time=time,
                channel=channel,
                waveform=waveform,
                reverse_polarity=reverse_polarity,
                wait_for_completion=False,
            )
            ids.append(id)

        if wait_for_completion:
            self._wait_for_completions(ids=ids)

        return ids

    def send_immediate_default_pulse_to_all_channels(self, reverse_polarities, wait_for_completion=True):
        time = self.get_time() + self.TIME_EPSILON

        ids = self.send_timed_default_pulse_to_all_channels(
            reverse_polarities=reverse_polarities,
            time=time,
            wait_for_completion=wait_for_completion,
        )
        return ids

    def send_charge_or_discharge(self, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0.0, wait_for_completion=True):
        voltage = self.get_voltage(channel=channel)
        charge_or_discharge = self.send_charge if voltage < target_voltage else self.send_discharge

        id = charge_or_discharge(
            channel=channel,
            target_voltage=target_voltage,
            execution_condition=execution_condition,
            time=time,
            wait_for_completion=wait_for_completion,
        )

        return id

    # MEP analysis

    def analyze_mep(self, time, emg_channel, mep_configuration):
        amplitude, latency, errors = self.node.analyze_mep(
            time=time,
            emg_channel=emg_channel,
            mep_configuration=mep_configuration,
        )

        # HACK: ROS2 doesn't support NaN values, therefore they are encoded as 0.0 values. Do the decoding
        #   here. Maybe it would better to use non-zero errors instead as criterion for returning Nones?
        #
        if amplitude == 0.0:
            amplitude = None
        if latency == 0.0:
            latency = None

        return amplitude, latency, errors

    # Other

    def print_system_state(self):
        self.node.wait_for_new_state()

    def get_wallclock_time(self):
        return time.time()

    def end(self):
        self.node.destroy_node()
        rclpy.shutdown()
