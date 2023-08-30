% This Matlab module provides an API for controlling a multi-channel transcranial magnetic stimulation (mTMS) device.
% It includes the functionality to start and stop the device, send pulses, charges, and discharges, and to perform 
% various analyses of the EEG/EMG data.
%
% It uses the Robot Operating System (ROS2) to interact with the device.

classdef MTMSApi < handle
    % An API for controlling a multi-channel transcranial magnetic stimulation (mTMS) device.
    
    properties (Constant)
        % TODO: Channel count hardcoded for now.
        N_CHANNELS = 5

        % TIME_EPSILON is used to implement events that are to be executed immediately but
        % wanting to synchronize them: to do that, get current time, add TIME_EPSILON to it,
        % and execute all the events at that time.
        %
        % Consequently, TIME_EPSILON must be large enough to allow time to send the events to
        % the mTMS device, but not too large so that the events are not executed 'immediately'.
        %
        % TODO: In MATLAB, 0.1 seconds is too little. In Python, however, 0.1 s works fine.
        %   Settle for 1.0 s for now, but it could be investigated if the code could be optimized
        %   to allow a shorter time interval.
        %
        TIME_EPSILON = 1.0
    end

    properties
        node
        enums

        device_states
        experiment_states
        execution_conditions

        event_id
    end

    methods

        function obj = MTMSApi()
        % Initializes the MTMSApi object. Does not require any parameters.
        %     
        % :returns: An MTMSApi object with five properties:
        %
        %   * 'node' - An instance of the MTMSApiNode class.
        %   * 'event_id' - A numerical ID, initialized to 0.
        %   * 'device_states' - A ROS2 message object, of type "mtms_device_interfaces/DeviceState".
        %   * 'experiment_states' - A ROS2 message object, of type "mtms_device_interfaces/ExperimentState".
        %   * 'execution_conditions' - A ROS2 message object, of type "event_interfaces/ExecutionCondition".

            obj.node = MTMSApiNode();

            obj.event_id = 0;

            obj.device_states = ros2message("mtms_device_interfaces/DeviceState");
            obj.experiment_states = ros2message("mtms_device_interfaces/ExperimentState");

            obj.execution_conditions = ros2message("event_interfaces/ExecutionCondition");
        end

        % General

        function event_id = next_event_id(obj)
        % Increment the event ID and return the new ID. 
        %
        % :returns: The incremented event ID.
        % :rtype: int

            obj.event_id = obj.event_id + 1;
            event_id = obj.event_id;
        end

        % Start and stop

        function start_device(obj)
        % Start the mTMS device, waiting until the device reports its state as operational.
        % Does not require any parameters. Does not return any value.

            while obj.get_device_state() ~= obj.device_states.OPERATIONAL
    
            end
        end
    
        function stop_device(obj)
        % Stop the mTMS device, waiting until the device reports its state as non-operational.
        % Does not require any parameters. Does not return any value.

            obj.node.stop_device();
            while obj.get_device_state() ~= obj.device_states.NOT_OPERATIONAL
    
            end
        end
    
        function start_experiment(obj)
        % Start an experiment, waiting until the experiment state is reported as started.
        % Does not require any parameters. Does not return any value.

            obj.node.start_experiment();
            while obj.get_experiment_state() ~= obj.experiment_states.STARTED
    
            end
        end
    
        function stop_experiment(obj)
        % Stop an experiment, waiting until the experiment state is reported as stopped.
        % Does not require any parameters. Does not return any value.

            while obj.get_experiment_state() ~= obj.experiment_states.STOPPED
    
            end
        end
    
        % Wait
    
        function wait_forever(obj)
        % Continuously wait for a new system state indefinitely.
        % Does not require any parameters. Does not return any value.

            while true
                obj.node.wait_for_new_state();
            end
        end
    
        function wait_until(obj, time)
        % Wait until the system time is equal to or greater than the specified time.
        % 
        % :param time: The time until which the function should wait.
        % :type time: float

            obj.node.wait_for_new_state();
            while obj.get_time() < time
                obj.node.wait_for_new_state();
            end
        end
    
        function wait_for_completion(obj, id)
        % Wait until the completion of an event with a given ID.
        %
        % :param id: The ID of the event to wait for.
        % :type id: int

            obj.node.wait_for_new_state();
            while ~isstruct(obj.node.get_event_feedback(id))
                obj.node.wait_for_new_state();
            end
        end
    
        function wait_for_completions(obj, ids)
        % Wait until the completion of events with given IDs.
        %
        % :param ids: The ids of the events to wait for.
        % :type ids: list of ints

        for i = 1:length(ids)
                obj.wait_for_completion(ids(i));
            end
        end
    
        function wait(obj, time)
        % Wait for a specified amount of time.
        % 
        % :param time: The time to wait.
        % :type time: float

            start_time = obj.get_wallclock_time();
    
            obj.node.wait_for_new_state();
            
            while obj.get_wallclock_time() < start_time + seconds(time)
                obj.node.wait_for_new_state();
            end
        end
    
        % Getters
    
        function state = get_device_state(obj)
        % Return the current state of the device.
        % Does not require any parameters.
        % 
        % :return: The current state of the device. One of the following
        %
        %   * DeviceState.NOT_OPERATIONAL : Device is unoperational.
        %   * DeviceState.STARTUP : Device is starting up.
        %   * DeviceState.OPERATIONAL : Device is operational.
        %   * DeviceState.SHUTDOWN : Device is shutting down.
        % :rtype: int 

            obj.node.wait_for_new_state();
            state = obj.node.system_state.device_state.value;
        end
    
        function state = get_experiment_state(obj)
        % Return the current state of the experiment.
        % Does not require any parameters.
        %
        % :return: The current state of the experiment. One of the following
        %
        %   * ExperimentState.STOPPED : Experiment is stopped.
        %   * ExperimentState.STARTING : Experiment is starting.
        %   * ExperimentState.STARTED : Experiment is started.
        %   * ExperimentState.STOPPING : Experiment is stopping.
        % :rtype: int

            obj.node.wait_for_new_state();
            state = obj.node.system_state.experiment_state.value;
        end
    
        function voltage = get_voltage(obj, channel)
        % Return the capacitor voltage (V) of the given channel.
        % 
        % :param channel: The channel ID.
        % :type channel: int
        % :return: The capacitor voltage (V) of the specified channel.
        % :rtype: float

            obj.node.wait_for_new_state();
            voltage = obj.node.system_state.channel_states(channel).voltage;
        end
    
        function temperature = get_temperature(obj, channel)
        % Return the coil temperature of the given channel if a temperature sensor is present, 
        % otherwise return None.
        % 
        % :param channel: The channel ID.
        % :type channel: int
        % :return: The coil temperature of the specified channel.
        % :rtype: float
            obj.node.wait_for_new_state();
            temperature = obj.node.system_state.channel_states(channel).temperature;
        end
    
        function pulse_count = get_pulse_count(obj, channel)
        % Return the total number of pulses generated with the coil connected to the specified channel.
        % 
        % :param channel: The channel ID.
        % :type channel: int
        % :return: The total number of pulses generated.
        % :rtype: float

            obj.node.wait_for_new_state();
            pulse_count = obj.node.system_state.channel_states(channel).pulse_count;
        end
    
        function time = get_time(obj)
        % Return the current time from the start of the experiment.
        % 
        % :return: The current time as seconds.
        % :rtype: float

            obj.node.wait_for_new_state();
            time = obj.node.system_state.time;
        end
    
        function feedback = get_event_feedback(obj, id)
        % Return feedback for a specified event id.
        % 
        % :param channel: The event ID.
        % :type channel: int
        % :return: The feedback.
        % :rtype: int    
    
            feedback = obj.node.get_event_feedback(id);
        end
    
        % Events

        function started = is_experiment_started(obj)
        % Return whether experiment has started (true or false).
        % 
        % :return: Whether experiment has started.
        % :rtype: bool 

            started = obj.get_experiment_state() == obj.experiment_states.STARTED;
        end
    
        function id = send_pulse(obj, channel, waveform, execution_condition, time, reverse_polarity, wait_for_completion)
        % Send a pulse event to a specified channel.
        %
        % :param channel: The target channel. Range: 1-5
        % :type channel: int
        % :param waveform: A list of dictionaries with keys `mode` and `duration_in_ticks`:
        %
        %   * `mode` is one of the following:
        %   * PulseMode.RISING
        %   * PulseMode.HOLD
        %   * PulseMode.FALLING
        %   * PulseMode.ALTERNATIVE_HOLD
        %   * `duration_in_ticks`, range: 0-65535
        %
        % :type waveform: list of dictionaries
        % :param execution_condition: The condition under which the event should be executed. One of the following:
        %
        %   * ExecutionCondition.IMMEDIATE : Execute the event immediately.
        %   * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
        %   * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.
        %
        %   Default is ExecutionCondition.TIMED
        % :type execution_condition: ExecutionCondition, optional
        % :param time: The time at which the pulse should be sent. Default is 0.0.
        % :type time: float, optional
        %
        % :param reverse_polarity: Whether to reverse the polarity of the waveform. Default is false.
        % :type reverse_polarity: bool, optional        
        % :param wait_for_completion: Whether to wait for the pulse to complete before returning. Default is true.
        % :type wait_for_completion: bool, optional
        % :return: The ID of the event.
        % :rtype: int
        %   
        % .. note:: The event ID is incremented with each pulse sent.

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            id = obj.next_event_id();

            waveform_ = waveform;
            if reverse_polarity
                waveform_ = obj.reverse_polarity(waveform);
            end

            obj.node.send_pulse(id, execution_condition, time, channel, waveform_);

            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end
    
        function id = send_charge(obj, channel, target_voltage, execution_condition, time, wait_for_completion)
        % Send a charge to a specified channel.
        %
        % :param channel: The channel for charging. Range: 1-5
        % :type channel: int
        % :param target_voltage: The target voltage for charging. Range: 0-1500
        % :type target_voltage: float
        % :param execution_condition: The condition under which the event should be executed. One of the following:
        %
        %   * ExecutionCondition.IMMEDIATE : Execute the event immediately.
        %   * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
        %   * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.
        %
        %   Default is ExecutionCondition.TIMED
        % :type execution_condition: ExecutionCondition, optional
        % :param time: The desired time for executing the event. Only used if execution_condition is ExecutionCondition.TIMED. Default is 0.0.
        % :type time: float, optional
        % :param wait_for_completion: Whether to wait for the charge to complete before returning. Default is true.
        % :type wait_for_completion: bool, optional
        %
        % :return:  The ID of the event.
        % :rtype: int
        %
        % .. note:: The event ID is incremented with each charge sent.

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            id = obj.next_event_id();

            obj.node.send_charge(id, execution_condition, time, channel, target_voltage);
    
            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end

        function id = send_discharge(obj, channel, target_voltage, execution_condition, time, wait_for_completion)
        % Send a discharge to a specified channel.
        %
        % :param channel: The channel for discharging. Range: 0-5
        % :type channel: int
        % :param target_voltage: The target voltage for the discharge.
        % :type target_voltage: float
        % :param execution_condition: The condition under which the event should be executed. One of the following:
        %
        %   * ExecutionCondition.IMMEDIATE : Execute the event immediately.
        %   * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
        %   * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.
        %
        %   Default is ExecutionCondition.TIMED
        % :type execution_condition: ExecutionCondition, optional
        % :param time: The desired time for executing the event. Only used if execution_condition is ExecutionCondition.TIMED. Default is 0.0.
        % :type time: float, optional
        % :param wait_for_completion: Whether to wait for the discharge to complete before returning. Default is true.
        % :type wait_for_completion: bool, optional
        %
        % :return: The ID of the event.
        % :rtype: int
        %
        % .. note:: The event ID is incremented with each discharge sent.

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            id = obj.next_event_id();
    
            obj.node.send_discharge(id, execution_condition, time, channel, target_voltage);
    
            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end
    
        function id = send_trigger_out(obj, port, duration_us, execution_condition, time, wait_for_completion)
        % Sends a trigger output to a specified port.
        %
        % :param port: The port number to send the trigger output to.
        % :type port: int
        % :param duration_us: The duration of the trigger in microseconds.
        % :type duration_us: int
        % :param execution_condition: The condition under which the event should be executed. One of the following:
        %
        %   * ExecutionCondition.IMMEDIATE : Execute the event immediately.
        %   * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
        %   * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.
        %
        %   Default is ExecutionCondition.TIMED
        % :type execution_condition: ExecutionCondition, optional
        % :param time: The time at which the trigger should be sent. Default is 0.0.
        % :type time: float, optional
        % :param wait_for_completion: Whether to wait for the trigger to complete before returning. Default is true.
        % :type wait_for_completion: bool, optional
        %
        % :return: The ID of the event.
        % :rtype: int
        %
        % .. note:: The event ID is incremented with each trigger sent.

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            id = obj.next_event_id();

            obj.node.send_trigger_out(id, execution_condition, time, port, duration_us);
    
            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end
    
        function send_event_trigger(obj)
        % Executes the events which have execution_condition set to ExecutionCondition.TRIGGERED.
        %
        % Does not require any parameters. Does not return any value.
        
            obj.node.send_event_trigger();
        end
    
        % Waveforms and targeting

        function waveform = create_waveform(obj, phases, durations_in_ticks)
            assert(iscell(phases), "Phases must be given as a cell array, e.g., {'RISING', 'HOLD', 'FALLING'}.");
            assert(iscell(durations_in_ticks), "Durations must be given as a cell array, e.g., {1200, 2400, 1200}.");

            assert(length(phases) == length(durations_in_ticks), 'Length of phases must be equal to the length of durations.');

            waveform = [];
            for i = 1:length(phases)
                phase = phases{i};
                duration_in_ticks = durations_in_ticks{i};

                allowed_phases = {'NON_CONDUCTIVE', 'RISING', 'HOLD', 'FALLING', 'ALTERNATIVE_HOLD'};
                assert(ismember(phase, allowed_phases), ['Phase must be one of ' strjoin(allowed_phases, ', ')]);

                piece = ros2message('event_interfaces/WaveformPiece');
                piece.duration_in_ticks = duration_in_ticks;

                piece.waveform_phase.value = piece.waveform_phase.(phase);

                waveform = [waveform piece];
            end
        end

        function waveform = get_default_waveform(obj, channel)
            waveform = obj.node.get_default_waveform(channel);
        end
    
        function waveform = reverse_polarity(obj, waveform)
            waveform = obj.node.reverse_polarity(waveform);
        end
    
        function [voltages, reverse_polarities] = get_channel_voltages(obj, displacement_x, displacement_y, rotation_angle, intensity)
        % Return the channel voltages (V) given the displacements, rotation angle and intensity.
        %
        % :param displacement_x: Displacement in the x direction.
        % :type displacement_x: float
        % :param displacement_y: Displacement in the y direction.
        % :type displacement_y: float
        % :param rotation_angle: Rotation angle in degrees.
        % :type rotation_angle: float
        % :param intensity: Intensity value.
        % :type intensity: float
        %
        % :return: Channel voltages.
        % :rtype: list of floats

            [voltages, reverse_polarities] = obj.node.get_channel_voltages(displacement_x, displacement_y, rotation_angle, intensity);
        end

        function maximum_intensity = get_maximum_intensity(obj, displacement_x, displacement_y, rotation_angle)
        % Return the maximum intensity given the displacements and rotation angle.
        %
        % :param displacement_x: Displacement in the x direction.
        % :type displacement_x: float
        % :param displacement_y: Displacement in the y direction.
        % :type displacement_y: float
        % :param rotation_angle: Rotation angle in degrees.
        % :type rotation_angle: float
        %
        % :return: The maximum intensity.
        % :rtype: float

            maximum_intensity = obj.node.get_maximum_intensity(displacement_x, displacement_y, rotation_angle);
        end

        % Other

        function [amplitude, latency, errors] = analyze_mep(obj, emg_channel, time, mep_configuration)
        % Analyze an MEP (motor evoked potential) by passing the time, EMG (electromyogram) channel, and MEP configuration.
        %
        % :param time: Time point to analyze the MEP.
        % :type time: float
        % :param emg_channel: Channel number for the EMG.
        % :type emg_channel: int
        % :param mep_configuration: Configuration object for the MEP.
        % :type mep_configuration: object
        %
        % :return: A list containing the amplitude, latency, and any errors encountered during the analysis.
        % :rtype: list

            [amplitude, latency, errors] = obj.node.analyze_mep(emg_channel, time, mep_configuration);

            % HACK: ROS2 doesn't support NaN in float64 type, work around by using 0.0 instead of NaN in message; map to NaN here.
            if amplitude == 0.0
                amplitude = NaN;
            end
            if latency == 0.0
                amplitude = NaN;
            end
        end

        function mep_configuration = create_mep_configuration(obj, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit)
            % FIXME: Placeholder
            % Create an MEP configuration given the start and end of time window, and four parameters
            % defining the preactivation check: whether the check is enabled, start and end time for the check, and preactivation voltage range limit.
            %
            % :param mep_start_time: Start of the configuration time window.
            % :type mep_start_time: float
            % :param mep_end_time: End of the configuration time window.
            % :type mep_end_time: float
            % :param preactivation_check_enabled: Whether to enable the check.
            % :type preactivation_check_enabled: bool
            % :param preactivation_start_time: Start of the preactivation.
            % :type mep_start_time: float
            % :param preactivation_end_time: End of the preactivation.
            % :type mep_end_time: float
            % :param preactivation_voltage_range_limit: Voltage range limit.
            % :type mep_end_time: float
            % 
            % :return: MEP configuration
            % :rtype: ???
            
            mep_configuration = ros2message('mep_interfaces/MepConfiguration');
            
            mep_configuration.time_window.start = mep_start_time;
            mep_configuration.time_window.end = mep_end_time;
            
            preactivation_check = ros2message('mep_interfaces/PreactivationCheck');
            
            preactivation_check.enabled = preactivation_check_enabled;
            preactivation_check.time_window.start = preactivation_start_time;
            preactivation_check.time_window.end = preactivation_end_time;
            preactivation_check.voltage_range_limit = preactivation_voltage_range_limit;
            
            mep_configuration.preactivation_check = preactivation_check;
        end

        % Compound events

        function id = send_charge_or_discharge(obj, channel, target_voltage, execution_condition, time, wait_for_completion)
        % Send charge or discharge command to a specified channel based on the current and target voltage.
        %
        % :param channel: Channel number.
        % :type channel: int
        % :param target_voltage: Target voltage for the channel.
        % :type target_voltage: float
        % :param execution_condition: The condition under which the event should be executed. One of the following:
        %
        %   * ExecutionCondition.IMMEDIATE : Execute the event immediately.
        %   * ExecutionCondition.TIMED : Execute the event when the desired time is reached.
        %   * ExecutionCondition.TRIGGERED : Execute the event when an external trigger is sent or a trigger command is sent.
        %
        %   Default is ExecutionCondition.TIMED
        % :type execution_condition: ExecutionCondition, optional
        % :param time: Time delay before sending the pulse, by default 0.0.
        % :type time: float, optional
        % :param wait_for_completion: Whether to wait for the completion of the command, by default true.
        % :type wait_for_completion: bool, optional
        %
        % :return: ID of the sent command.
        % :rtype: int

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            voltage = obj.get_voltage(channel);
            if voltage < target_voltage
                id = obj.send_charge(channel, target_voltage, execution_condition, time, wait_for_completion);
            else
                id = obj.send_discharge(channel, target_voltage, execution_condition, time, wait_for_completion);
            end
        end

        function ids = send_immediate_charge_or_discharge_to_all_channels(obj, target_voltages, wait_for_completion)
        % Send immediate charge or discharge commands to all channels.
        %
        % :param target_voltages: List of target voltages for each channel.
        % :type target_voltages: list of floats
        % :param wait_for_completion: Whether to wait for the completion of all commands, by default true.
        % :type wait_for_completion: bool, optional
        %
        % :return: list of event IDs for each sent command
        % :return type: list of ints

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            assert(length(target_voltages) == obj.N_CHANNELS, sprintf("Target voltage only defined for %d channels, channel count %d.", ...
                length(target_voltages), obj.N_CHANNELS));

            ids = [];
            for channel = 1:obj.N_CHANNELS
                target_voltage = target_voltages(channel);

                new_id = obj.send_charge_or_discharge(channel, target_voltage, obj.execution_conditions.IMMEDIATE, 0.0, false);    
                ids = [ids new_id];
            end
    
            if wait_for_completion
                obj.wait_for_completions(ids);
            end
        end
    
        function ids = send_immediate_full_discharge_to_all_channels(obj, wait_for_completion)
        % Send immediate full discharge commands to all channels.
        %
        % :param wait_for_completion: Whether to wait for the completion of all commands, by default true.
        % :type wait_for_completion: bool, optional
        %
        % :return: IDs for each sent command.
        % :rtype: list of ints
        
            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            target_voltages = zeros(1, obj.N_CHANNELS);
    
            ids = obj.send_immediate_charge_or_discharge_to_all_channels(target_voltages, wait_for_completion);
        end

        function ids = send_timed_default_pulse_to_all_channels(obj, reverse_polarities, time, wait_for_completion)
        % Send timed default pulse commands to all channels.
        %
        % :param reverse_polarities: List of boolean values indicating whether to reverse polarities for each channel.
        % :type reverse_polarities: list of bools
        % :param time: Time delay before sending the pulse, by default 0.0.
        % :type time: float, optional
        % :param wait_for_completion: Whether to wait for the completion of all commands, by default true.
        % :type wait_for_completion: bool, optional
        %
        % :return: IDs for each sent command.
        % :rtype: list of ints

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            assert(length(reverse_polarities) == obj.N_CHANNELS, ...
                sprintf("Reverse polarities only defined for %d channels, channel count %d.", length(reverse_polarities), obj.N_CHANNELS));
    
            ids = [];
            for channel = 1:obj.N_CHANNELS
                reverse_polarity = reverse_polarities(channel);
                waveform = obj.get_default_waveform(channel);

                new_id = obj.send_pulse(channel, waveform, obj.execution_conditions.TIMED, time, reverse_polarity, false);
                ids = [ids new_id];
            end
    
            if wait_for_completion
                obj.wait_for_completions(ids);
            end
        end
    
        function ids = send_immediate_default_pulse_to_all_channels(obj, reverse_polarities, wait_for_completion)
        % Send immediate default pulse commands to all channels.
        %
        % :param reverse_polarities: List of boolean values indicating whether to reverse polarities for each channel.
        % :type reverse_polarities: list of bools
        % :param wait_for_completion: Whether to wait for the completion of all commands, by default true.
        % :type wait_for_completion: bool, optional
        %
        % :return: IDs for each sent command.
        % :rtype: list of ints

            assert(obj.is_experiment_started(), sprintf("Experiment not started."));

            time = obj.get_time() + obj.TIME_EPSILON;
    
            ids = obj.send_timed_default_pulse_to_all_channels(reverse_polarities, time, wait_for_completion);
        end
        
        % Other
    
        function print_system_state(obj)
            obj.node.wait_for_new_state()
        end
    
        function time = get_wallclock_time(obj)
            time = datetime("now");
        end
    end
end
