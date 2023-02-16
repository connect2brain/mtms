classdef MTMSApi < handle
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
            obj.node = MTMSApiNode();

            obj.event_id = 0;

            obj.device_states = ros2message("mtms_device_interfaces/DeviceState");
            obj.experiment_states = ros2message("mtms_device_interfaces/ExperimentState");

            obj.execution_conditions = ros2message("event_interfaces/ExecutionCondition");
        end

        % General

        function event_id = next_event_id(obj)
            obj.event_id = obj.event_id + 1;
            event_id = obj.event_id;
        end

        % Start and stop
    
        function start_device(obj)
            obj.node.start_device();
            while obj.get_device_state() ~= obj.device_states.OPERATIONAL
    
            end
        end
    
        function stop_device(obj)
            obj.node.stop_device();
            while obj.get_device_state() ~= obj.device_states.NOT_OPERATIONAL
    
            end
        end
    
        function start_experiment(obj)
            obj.node.start_experiment();
            while obj.get_experiment_state() ~= obj.experiment_states.STARTED
    
            end
        end
    
        function stop_experiment(obj)
            obj.node.stop_experiment();
            while obj.get_experiment_state() ~= obj.experiment_states.STOPPED
    
            end
        end
    
        % Wait
    
        function wait_forever(obj)
            while true
                obj.node.wait_for_new_state();
            end
        end
    
        function wait_until(obj, time)
            obj.node.wait_for_new_state();
            while obj.get_time() < time
                obj.node.wait_for_new_state();
            end
        end
    
        function wait_for_completion(obj, id)
            obj.node.wait_for_new_state();
            while ~isstruct(obj.node.get_event_feedback(id))
                obj.node.wait_for_new_state();
            end
        end
    
        function wait_for_completions(self, ids)
            for i = 1:length(ids)
                self.wait_for_completion(ids(i));
            end
        end
    
        function wait(obj, time)
            start_time = obj.get_wallclock_time();
    
            obj.node.wait_for_new_state();
            
            while obj.get_wallclock_time() < start_time + seconds(time)
                obj.node.wait_for_new_state();
            end
        end
    
        % Getters
    
        function state = get_device_state(obj)
            obj.node.wait_for_new_state();
            state = obj.node.system_state.device_state.value;
        end
    
        function state = get_experiment_state(obj)
            obj.node.wait_for_new_state();
            state = obj.node.system_state.experiment_state.value;
        end
    
        function voltage = get_voltage(obj, channel)
            obj.node.wait_for_new_state();
            voltage = obj.node.system_state.channel_states(channel).voltage;
        end
    
        function temperature = get_temperature(obj, channel)
            obj.node.wait_for_new_state();
            temperature = obj.node.system_state.channel_states(channel).temperature;
        end
    
        function pulse_count = get_pulse_count(obj, channel)
            obj.node.wait_for_new_state();
            pulse_count = obj.node.system_state.channel_states(channel).pulse_count;
        end
    
        function time = get_time(obj)
            obj.node.wait_for_new_state();
            time = obj.node.system_state.time;
        end
    
        function feedback = get_event_feedback(obj, id)
            feedback = obj.node.get_event_feedback(id);
        end
    
        % Events
    
        function id = send_pulse(obj, channel, waveform, execution_condition, time, reverse_polarity, wait_for_completion)
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
            id = obj.next_event_id();

            obj.node.send_charge(id, execution_condition, time, channel, target_voltage);
    
            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end

        function id = send_discharge(obj, channel, target_voltage, execution_condition, time, wait_for_completion)
            id = obj.next_event_id();
    
            obj.node.send_discharge(id, execution_condition, time, channel, target_voltage);
    
            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end
    
        function id = send_trigger_out(obj, port, duration_us, execution_condition, time, wait_for_completion)
            id = obj.next_event_id();

            obj.node.send_trigger_out(id, execution_condition, time, port, duration_us);
    
            if wait_for_completion
                obj.wait_for_completion(id);
            end
        end
    
        function send_event_trigger(obj)
            obj.node.send_event_trigger();
        end
    
        % Waveforms and targeting
    
        function waveform = get_default_waveform(obj, channel)
            waveform = obj.node.get_default_waveform(channel);
        end
    
        function waveform = reverse_polarity(obj, waveform)
            waveform = obj.node.reverse_polarity(waveform);
        end
    
        function [voltages, reverse_polarities] = get_channel_voltages(obj, displacement_x, displacement_y, rotation_angle, intensity)
            [voltages, reverse_polarities] = obj.node.get_channel_voltages(displacement_x, displacement_y, rotation_angle, intensity);
        end

        function maximum_intensity = get_maximum_intensity(obj, displacement_x, displacement_y, rotation_angle)
            maximum_intensity = obj.node.get_maximum_intensity(displacement_x, displacement_y, rotation_angle);
        end

        % Other

        function [amplitude, latency, errors] = analyze_mep(obj, emg_channel, time, mep_configuration)
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
            voltage = obj.get_voltage(channel);
            if voltage < target_voltage
                id = obj.send_charge(channel, target_voltage, execution_condition, time, wait_for_completion);
            else
                id = obj.send_discharge(channel, target_voltage, execution_condition, time, wait_for_completion);
            end
        end

        function ids = send_immediate_charge_or_discharge_to_all_channels(obj, target_voltages, wait_for_completion)
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
            target_voltages = zeros(1, obj.N_CHANNELS);
    
            ids = obj.send_immediate_charge_or_discharge_to_all_channels(target_voltages, wait_for_completion);
        end

        function ids = send_timed_default_pulse_to_all_channels(obj, reverse_polarities, time, wait_for_completion)
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
