"""
This Python module provides an API for controlling a multi-channel transcranial magnetic stimulation (mTMS) device.
It includes the functionality to start and stop the device, send pulses, charges, and discharges, and to perform 
various analyses and manipulations of the data. 

It uses the Robot Operating System (ROS) to interact with the device.
"""
import time
import copy

import rclpy

from mtms_device_interfaces.msg import ExperimentState, DeviceState
from event_interfaces.msg import ExecutionCondition, WaveformPhase

from MTMSApiNode import MTMSApiNode

class MTMSApi:
    """
    An API for controlling a multi-channel transcranial magnetic stimulation (mTMS) device.
    """    
    N_CHANNELS = 5
    """
    Channel count. Hardcoded for now.
    """

    TIME_EPSILON = 0.1
    """
    Used to implement events that are to be executed immediately but
    wanting to synchronize them: to do that, get current time, add TIME_EPSILON to it,
    and execute all the events at that time.
    
    Consequently, TIME_EPSILON must be large enough to allow time to send the events to
    the mTMS device, but not too large so that the events are not executed 'immediately'.
    Settle for 0.1 s (100 ms) for now, but change if needed.
    """

    def __init__(self):
        """
        Initializes the MTMSApi instance, creating a new MTMSApiNode.

        Does not require any parameters, and does not return any value.
        """
        rclpy.init(args=None)
        self.node = MTMSApiNode()
        self.event_id = 0

    # General

    def _next_event_id(self):
        """
        Increment the event id and return the new id.

        Does not require any parameters, and does not return any value.
        """
        self.event_id += 1
        return self.event_id

    # Start and stop

    def start_device(self):
        """
        Start the mTMS device, waiting until the device reports its state as operational.

        Does not require any parameters, and does not return any value.
        """
        self.node.start_device()
        while self.get_device_state() != DeviceState.OPERATIONAL:
            pass

    def stop_device(self):
        """
        Stop the mTMS device, waiting until the device reports its state as not operational.

        Does not require any parameters, and does not return any value.
        """
        self.node.stop_device()
        while self.get_device_state() != DeviceState.NOT_OPERATIONAL:
            pass

    def start_experiment(self):
        """
        Start an experiment, waiting until the experiment state is reported as started.

        Does not require any parameters, and does not return any value.
        """
        self.node.start_experiment()
        while self.get_experiment_state() != ExperimentState.STARTED:
            pass

    def stop_experiment(self):
        """
        Stop the current experiment, waiting until the experiment state is reported as stopped.

        Does not require any parameters, and does not return any value.
        """
        self.node.stop_experiment()
        while self.get_experiment_state() != ExperimentState.STOPPED:
            pass

    # Wait

    def _wait_for_completion(self, id):
        """
        Wait until the completion of an event with a given id.

        Parameters
        ----------
        id : int
            The id of the event to wait for.
        """
        self.node.wait_for_new_state()
        while self.node.get_event_feedback(id) is None:
            self.node.wait_for_new_state()

    def _wait_for_completions(self, ids):
        """
        Wait until the completion of multiple events given their ids.

        Parameters
        ----------
        ids : list of int
            The ids of the events to wait for.
        """
        for id in ids:
            self._wait_for_completion(id)

    def wait_forever(self):
        """
        Continuously wait for a new system state indefinitely.
        """
        while True:
            self.node.wait_for_new_state()

    def wait_until(self, time):
        """
        Wait until the system time is equal to or greater than the specified time.

        Parameters
        ----------
        time : float
            The time to wait until (in seconds).
        """
        self.node.wait_for_new_state()
        while self.get_time() < time:
            self.node.wait_for_new_state()

    def wait(self, time):
        """
        Wait for a specified duration of time.

        Parameters
        ----------
        time : float
            The duration to wait for (in seconds).
        """
        start_time = self.get_wallclock_time()

        self.node.wait_for_new_state()
        while self.get_wallclock_time() < start_time + time:
            self.node.wait_for_new_state()

    # Getters

    def get_device_state(self):
        """
        Return the current state of the device.

        Returns
        -------
        ???
            The current state of the device. One of the following:

            * DeviceState.NOT_OPERATIONAL : Device is unoperational.
            * DeviceState.STARTUP : Device is starting up.
            * DeviceState.OPERATIONAL : Device is operational.
            * DeviceState.SHUTDOWN : Device is shutting down.

        """
        self.node.wait_for_new_state()
        return self.node.system_state.device_state.value

    def get_experiment_state(self):
        """
        Return the current state of the experiment.

        Returns
        -------
        ???
            The current state of the experiment. One of the following:

            * ExperimentState.STOPPED : Experiment is stopped.
            * ExperimentState.STARTING : Experiment is starting.
            * ExperimentState.STARTED : Experiment is started.
            * ExperimentState.STOPPING : Experiment is stopping.
        """
        self.node.wait_for_new_state()
        return self.node.system_state.experiment_state.value

    def get_voltage(self, channel):
        """
        Return the capacitor voltage (V) of the given channel. 

        Parameters
        ----------
        channel : int
            The channel id.

        Returns
        -------
        float
            The capacitor voltage (V) of the specified channel.
        """
        self.node.wait_for_new_state()
        return self.node.system_state.channel_states[channel - 1].voltage

    def get_temperature(self, channel):
        """
        Return the coil temperature of the given channel if a temperature sensor is present, 
        otherwise return None.

        Parameters
        ----------
        channel : int
            The channel id.

        Returns
        -------
        float
            The coil temperature of the specified channel.
        """
        self.node.wait_for_new_state()
        return self.node.system_state.channel_states[channel - 1].temperature

    def get_pulse_count(self, channel):
        """
        Return the total number of pulses generated with the coil connected to the specified channel.

        Parameters
        ----------
        channel : int
            The channel id.

        Returns
        -------
        float
            The total number of pulses generated.
        """
        self.node.wait_for_new_state()
        return self.node.system_state.channel_states[channel - 1].pulse_count

    def get_time(self):
        """
        Return the current time. ???

        Returns
        -------
        int
            The current time as seconds.
        """
        self.node.wait_for_new_state()
        return self.node.system_state.time

    def get_event_feedback(self, id):
        """
        Return feedback for a specified event id.

        Parameters
        ----------
        id : int
            The ID of the event.

        Returns
        -------
        int
            The event feedback.
        """
        return self.node.get_event_feedback(id)

    # Events
    def send_pulse(self, channel, waveform, execution_condition=ExecutionCondition.TIMED, time=0.0, reverse_polarity=False, wait_for_completion=True):
        """
        Send a pulse event to a specific channel.

        Parameters
        ----------
        channel : int
            The target channel. Range: 1-5
        waveform : list of dicts
            A list of dictionaries with keys `mode` and `duration_in_ticks`:

                * `mode` is one of the following:
                    * PulseMode.RISING
                    * PulseMode.HOLD
                    * PulseMode.FALLING
                    * PulseMode.ALTERNATIVE_HOLD
                * `duration_in_ticks`, range: 0-65535

        execution_condition : ExecutionCondition, optional
            The condition under which the event should be executed. One of the following:
            
            * ExecutionCondition.INSTANT : Execute the event instantly.
            * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
            * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.

            Default is ExecutionCondition.TIMED.
        time : float, optional
            The time at which the pulse should be sent. Default is 0.0.
        reverse_polarity : bool, optional
            Whether to reverse the polarity of the waveform. Default is False.
        wait_for_completion : bool, optional
            Whether to wait for the pulse to complete before returning. Default is True.

        Returns
        -------
        int
            The ID of the event.

        Notes
        -----
        The event ID is incremented with each pulse sent.
        """
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
        """
        Send a charge to a specific channel.

        Parameters
        ----------
        channel : int
            The channel for charging. Range: 1-5
        target_voltage : float
            The target voltage for charging. Range: 0-1500
        execution_condition : ExecutionCondition, optional
            The condition under which the event should be executed. One of the following:
            
            * ExecutionCondition.INSTANT : Execute the event instantly.
            * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
            * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.

            Default is ExecutionCondition.TIMED.
        time : float, optional
            The desired time for executing the event. Only used if execution_condition is ExecutionCondition.TIMED. Default is 0.0.
        wait_for_completion : bool, optional
            Whether to wait for the charge to complete before returning. Default is True.

        Returns
        -------
        int
            The ID of the event.

        Notes
        -----
        The event ID is incremented with each charge sent.
        """
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
        """
        Send a discharge to a specific channel.

        Parameters
        ----------
        channel : int
            The channel for discharging. Range: 0-5
        target_voltage : float
            The target voltage for the discharge.
        execution_condition : ExecutionCondition, optional
            The condition under which the event should be executed. One of the following:
            
            * ExecutionCondition.INSTANT : Execute the event instantly.
            * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
            * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.

            Default is ExecutionCondition.TIMED.
        time : float, optional
            The desired time for executing the event. Only used if execution_condition is ExecutionCondition.TIMED. Default is 0.0.
        wait_for_completion : bool, optional
            Whether to wait for the discharge to complete before returning. Default is True.

        Returns
        -------
        int
            The ID of the event.

        Notes
        -----
        The event ID is incremented with each discharge sent.
        """
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
        """
        Sends a trigger output to a specific port.

        Parameters
        ----------
        port : int
            The port number to send the trigger output to.
        duration_us : int
            The duration of the trigger in microseconds.
        execution_condition : ExecutionCondition, optional
            The condition under which the event should be executed. One of the following:
            
            * ExecutionCondition.INSTANT : Execute the event instantly.
            * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
            * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.

            Default is ExecutionCondition.TIMED.
        time : float, optional
            The time at which the trigger should be sent. Default is 0.0.
        wait_for_completion : bool, optional
            Whether to wait for the trigger to complete before returning. Default is True.

        Returns
        -------
        int
            The ID of the event.

        Notes
        -----
        The event ID is incremented with each trigger sent.
        """
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
        """
        Executes the events which have execution_condition set to ExecutionCondition.TRIGGERED.

        Does not require any parameters, and does not return any value.
        """
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
        """
        Return the channel voltages (V) given the displacements, rotation angle and intensity.

        Parameters
        ----------
        displacement_x : float
            Displacement in the x direction.
        displacement_y : float
            Displacement in the y direction.
        rotation_angle : float
            Rotation angle in degrees.
        intensity : float
            Intensity value.

        Returns
        -------
        array-like
            Channel voltages.
        """        
        return self.node.get_channel_voltages(
            displacement_x=displacement_x,
            displacement_y=displacement_y,
            rotation_angle=rotation_angle,
            intensity=intensity,
        )

    def get_maximum_intensity(self, displacement_x, displacement_y, rotation_angle):
        """
        Return the maximum intensity given the displacements and rotation angle.

        Parameters
        ----------
        displacement_x : float
            Displacement in the x direction.
        displacement_y : float
            Displacement in the y direction.
        rotation_angle : float
            Rotation angle in degrees.

        Returns
        -------
        float
            The maximum intensity.
        """
        
        return self.node.get_maximum_intensity(
            displacement_x=displacement_x,
            displacement_y=displacement_y,
            rotation_angle=rotation_angle,
        )

    # Compound events

    def send_immediate_charge_or_discharge_to_all_channels(self, target_voltages, wait_for_completion=True):
        """
        Send immediate charge or discharge commands to all channels.

        Parameters
        ----------
        target_voltages : list of floats
            List of target voltages for each channel.
        wait_for_completion : bool, optional
            Whether to wait for the completion of all commands, by default True.

        Returns
        -------
        list
            IDs for each sent command.
        """
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
        """
        Send immediate full discharge commands to all channels.

        Parameters
        ----------
        wait_for_completion : bool, optional
            Whether to wait for the completion of all commands, by default True.

        Returns
        -------
        list
            IDs for each sent command.
        """
        target_voltages = self.N_CHANNELS * [0]

        ids = self.send_immediate_charge_or_discharge_to_all_channels(
            target_voltages=target_voltages,
            wait_for_completion=wait_for_completion,
        )

        return ids

    def send_timed_default_pulse_to_all_channels(self, reverse_polarities, time=0.0, wait_for_completion=True):
        """
        Send timed default pulse commands to all channels.

        Parameters
        ----------
        reverse_polarities : list of bools
            List of boolean values indicating whether to reverse polarities for each channel.
        time : float, optional
            Time delay before sending the pulse, by default 0.0.
        wait_for_completion : bool, optional
            Whether to wait for the completion of all commands, by default True.

        Returns
        -------
        list
            IDs for each sent command.
        """
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
        """
        Send immediate default pulse commands to all channels.

        Parameters
        ----------
        reverse_polarities : list of bools
            List of boolean values indicating whether to reverse polarities for each channel.
        wait_for_completion : bool, optional
            Whether to wait for the completion of all commands, by default True.

        Returns
        -------
        list
            IDs for each sent command.
        """
        time = self.get_time() + self.TIME_EPSILON

        ids = self.send_timed_default_pulse_to_all_channels(
            reverse_polarities=reverse_polarities,
            time=time,
            wait_for_completion=wait_for_completion,
        )
        return ids

    def send_charge_or_discharge(self, channel, target_voltage, execution_condition=ExecutionCondition.TIMED, time=0.0, wait_for_completion=True):
        """
        Send charge or discharge command to a specified channel based on the current and target voltage.

        Parameters
        ----------
        channel : int
            Channel number.
        target_voltage : float
            Target voltage for the channel.
        execution_condition : ExecutionCondition, optional
            The condition under which the event should be executed. One of the following:
            
            * ExecutionCondition.INSTANT : Execute the event instantly.
            * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
            * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.

            Default is ExecutionCondition.TIMED.
        time : float, optional
            Time delay before sending the pulse, by default 0.0.
        wait_for_completion : bool, optional
            Whether to wait for the completion of the command, by default True.

        Returns
        -------
        int
            ID of the sent command.
        """
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
        """
        Analyze the MEP (Motor Evoked Potentials) by passing the time, EMG (Electromyogram) channel, and MEP configuration.

        Parameters
        ----------
        time : float
            Time point to analyze the MEP.
        emg_channel : int
            Channel number for the EMG.
        mep_configuration : object
            Configuration object for the MEP.

        Returns
        -------
        tuple
            A tuple containing the amplitude, latency, and any errors encountered during the analysis.
        """
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
        """
        Destroy node. ???

        Does not require any parameters, and does not return any value.
        """
        self.node.destroy_node()
        rclpy.shutdown()
