classdef MTMSApiNode < handle
    %MTMSApiNode A class for communicating with other ROS nodes.

    properties
        node
        printer
        system_state
        session

        event_feedback

        start_device_client
        stop_device_client
        start_session_client
        stop_session_client

        request_trigger_client

        allow_stimulation_client
        request_events_client

        system_state_subscriber
        session_subscriber

        pulse_feedback_subscriber
        trigger_out_feedback_subscriber
        charge_feedback_subscriber
        discharge_feedback_subscriber

        create_marker_publisher

        decider_enabled_subscriber

        get_target_voltages_client
        get_maximum_intensity_client
        get_default_waveform_client
        get_multipulse_waveforms_client
        reverse_polarity_client

        analyze_mep_client
    end

    methods
        function obj = MTMSApiNode(channel_count)

            % Currently in the Aalto mTMS lab, the API seems to work without changing
            % RMW_IMPLEMENTATION. Hence, comment out the line below. If in the
            % future it is required (e.g., due to updating to a more recent ROS
            % version), the line can be uncommented.
            %

            % setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp")

            obj.node = ros2node("mtms_matlab_api");
            obj.printer = MTMSApiPrinter(channel_count);

            obj.system_state = NaN;
            obj.session = NaN;
            obj.event_feedback = dictionary();

            % To mTMS device

            obj.start_device_client = ros2svcclient(obj.node, "/mtms/device/start_device", "mtms_device_interfaces/StartDevice");
            obj.stop_device_client = ros2svcclient(obj.node, "/mtms/device/stop_device", "mtms_device_interfaces/StopDevice");

            obj.allow_stimulation_client = ros2svcclient(obj.node, "/mtms/device/allow_stimulation", "mtms_device_interfaces/AllowStimulation");
            obj.request_events_client = ros2svcclient(obj.node, "/mtms/device/events/request", "mtms_device_interfaces/RequestEvents");

            obj.system_state_subscriber = ros2subscriber(obj.node, "/mtms/device/system_state", "mtms_device_interfaces/SystemState", @obj.handle_system_state);

            obj.request_trigger_client = ros2svcclient(obj.node, "/mtms_device/request_trigger", "mtms_device_interfaces/RequestTrigger");

            % System-related

            obj.session_subscriber = ros2subscriber(obj.node, "/mtms/device/session", "system_interfaces/Session", @obj.handle_session);

            obj.start_session_client = ros2svcclient(obj.node, "/mtms/device/session/start", "system_interfaces/StartSession");
            obj.stop_session_client = ros2svcclient(obj.node, "/mtms/device/session/stop", "system_interfaces/StopSession");

            % Event-related.
            obj.pulse_feedback_subscriber = ros2subscriber(obj.node, "/mtms/device/events/feedback/pulse", "event_msgs/PulseFeedback", @obj.handle_pulse_feedback);
            obj.charge_feedback_subscriber = ros2subscriber(obj.node, "/mtms/device/events/feedback/charge", "event_msgs/ChargeFeedback", @obj.handle_charge_feedback);
            obj.discharge_feedback_subscriber = ros2subscriber(obj.node, "/mtms/device/events/feedback/discharge", "event_msgs/DischargeFeedback", @obj.handle_discharge_feedback);
            obj.trigger_out_feedback_subscriber = ros2subscriber(obj.node, "/mtms/device/events/feedback/trigger_out", "event_msgs/TriggerOutFeedback", @obj.handle_trigger_out_feedback);

            obj.create_marker_publisher = ros2publisher(obj.node, "/neuronavigation/create_marker");

            % Decider.
            obj.decider_enabled_subscriber = ros2subscriber(...
                obj.node, ...
                '/pipeline/decider/enabled', ...
                'std_msgs/Bool', ...
                @obj.handle_decider_enabled, ...
                'Durability', 'transientlocal', ...
                'History', 'keeplast', ...
                'Depth', 1, ...
                'Reliability', 'reliable');

            % To other parts of the system.

            obj.get_target_voltages_client = ros2svcclient(obj.node, "/mtms/targeting/get_target_voltages", "targeting_services/GetTargetVoltages");
            obj.get_maximum_intensity_client = ros2svcclient(obj.node, "/mtms/targeting/get_maximum_intensity", "targeting_services/GetMaximumIntensity");
            obj.get_default_waveform_client = ros2svcclient(obj.node, "/mtms/waveforms/get_default", "targeting_services/GetDefaultWaveform");
            obj.get_multipulse_waveforms_client = ros2svcclient(obj.node, "/mtms/waveforms/get_multipulse_waveforms", "targeting_services/GetMultipulseWaveforms");
            obj.reverse_polarity_client = ros2svcclient(obj.node, "/mtms/waveforms/reverse_polarity", "targeting_services/ReversePolarity");

            obj.analyze_mep_client = ros2svcclient(obj.node, "/mtms/mep/analyze_service", "mep_interfaces/AnalyzeMepService");
        end

        % Starting and stopping

        function success = start_device(obj)
            %start_device Start mTMS device
            %
            %   Start the mTMS device.

            client = obj.start_device_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        function success = stop_device(obj)
            %stop_device Stop mTMS device
            %
            %   Stop the mTMS device.

            client = obj.stop_device_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        function success = start_session(obj)
            %start_session Start session on mTMS device
            %
            %   Start an session on the mTMS device.

            client = obj.start_session_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        function success = stop_session(obj)
            %stop_session Stop session on mTMS device
            %
            %   Stop an session on the mTMS device.

            client = obj.stop_session_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        % Events

        function request_trigger(obj)
            %request_trigger Request trigger from mTMS device
            %
            %   Request trigger from the mTMS device.

            client = obj.request_trigger_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        function success = allow_stimulation(obj, allow_stimulation)
            %allow_stimulation Allow or disallow stimulation.
            %
            %   Allow or disallow stimulation.

            client = obj.allow_stimulation_client;

            request = ros2message(client);
            request.allow_stimulation = allow_stimulation;

            response = call(client, request);
            success = response.success;
        end

        function send_pulse(obj, id, channel, waveform, execution_condition, time)
            %send_pulse Send pulse event
            %
            %   Send pulse event to the mTMS device.

            client = obj.request_events_client;

            request = ros2message(client);

            % XXX: Encode NaN as 0, as ROS2 messages do not support null values.
            if isnan(time)
                time = 0;
            end

            event_info = ros2message("event_msgs/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            pulse = ros2message("event_msgs/Pulse");
            pulse.event_info = event_info;
            pulse.channel = uint8(channel);
            pulse.waveform = waveform;

            request.pulses = [pulse];
            request.charges = struct([]);
            request.discharges = struct([]);
            request.trigger_outs = struct([]);

            response = call(client, request);
            success = response.success;

            assert(success, "Failed to request pulse event.");
        end

        function send_charge(obj, id, channel, target_voltage, execution_condition, time)
            %send_charge Send charge event
            %
            %   Sends charge event to the mTMS device.

            client = obj.request_events_client;

            request = ros2message(client);

            % XXX: Encode NaN as 0, as ROS2 messages do not support null values.
            if isnan(time)
                time = 0;
            end

            event_info = ros2message("event_msgs/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            charge.event_info = event_info;
            charge.channel = uint8(channel);
            charge.target_voltage = uint16(target_voltage);

            request.charges = [charge];
            request.pulses = struct([]);
            request.discharges = struct([]);
            request.trigger_outs = struct([]);

            response = call(client, request);
            success = response.success;

            assert(success, "Failed to request charge event.");
        end

        function send_discharge(obj, id, channel, target_voltage, execution_condition, time)
            %send_discharge Send discharge command to mTMS device
            %
            %   Sends discharge command to the mTMS device.

            client = obj.request_events_client;

            request = ros2message(client);

            % XXX: Encode NaN as 0, as ROS2 messages do not support null values.
            if isnan(time)
                time = 0;
            end

            event_info = ros2message("event_msgs/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            discharge.event_info = event_info;
            discharge.channel = uint8(channel);
            discharge.target_voltage = uint16(target_voltage);

            request.discharges = [discharge];
            request.charges = struct([]);
            request.pulses = struct([]);
            request.trigger_outs = struct([]);

            response = call(client, request);
            success = response.success;

            assert(success, "Failed to request discharge event.");
        end

        function send_trigger_out(obj, id, port, duration_us, execution_condition, time)
            %send_trigger_out Send trigger out event
            %
            %   Send trigger out event to the mTMS device.

            client = obj.request_events_client;

            request = ros2message(client);

            % XXX: Encode NaN as 0, as ROS2 messages do not support null values.
            if isnan(time)
                time = 0;
            end

            event_info = ros2message("event_msgs/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            trigger_out.event_info = event_info;
            trigger_out.port = uint8(port);
            trigger_out.duration_us = uint32(duration_us);

            request.trigger_outs = [trigger_out];
            request.charges = struct([]);
            request.pulses = struct([]);
            request.discharges = struct([]);

            response = call(client, request);
            success = response.success;

            assert(success, "Failed to request trigger out event.");
        end

        function create_marker(obj, create_marker_msg)
            %create_marker Send create_marker event
            %
            %   Publishes a CreateMarker message with create_marker_msg.

            pub = obj.create_marker_publisher;
            % Publish the message
            send(pub, create_marker_msg);
            % Log
            disp("Creating marker...");
        end

        % Feedback

        function update_event_feedback(obj, feedback)
            id = feedback.id;
            error = feedback.error;

            obj.event_feedback(id) = error;
        end

        function feedback = get_event_feedback(obj, id)
            feedback = NaN;

            % Check first if the dictionary is configured; if it is not,
            % implying that it is empty, isKey returns an error.
            if ~isConfigured(obj.event_feedback) || ~isKey(obj.event_feedback, id)
                return
            end

            feedback = obj.event_feedback(id);
        end


        function handle_pulse_feedback(obj, feedback)
            obj.printer.print_feedback('Pulse', feedback)
            obj.update_event_feedback(feedback)
        end

        function handle_trigger_out_feedback(obj, feedback)
            obj.printer.print_feedback('Trigger out', feedback)
            obj.update_event_feedback(feedback)
        end

        function handle_charge_feedback(obj, feedback)
            obj.printer.print_feedback('Charge', feedback)
            obj.update_event_feedback(feedback)
        end

        function handle_discharge_feedback(obj, feedback)
            obj.printer.print_feedback('Discharge', feedback)
            obj.update_event_feedback(feedback)
        end

        % Targeting

        function [voltages, reverse_polarities] = get_target_voltages(obj, target)

            client = obj.get_target_voltages_client;

            request = ros2message(client);

            request.target = target;

            response = call(client, request);
            success = response.success;

            assert(success, "Invalid displacement, rotation angle, or intensity.");

            voltages = response.voltages;
            reverse_polarities = response.reversed_polarities;
        end

        function maximum_intensity = get_maximum_intensity(obj, displacement_x, displacement_y, rotation_angle, algorithm)

            client = obj.get_maximum_intensity_client;

            request = ros2message(client);

            request.displacement_x = int8(displacement_x);
            request.displacement_y = int8(displacement_y);
            request.rotation_angle = int16(rotation_angle);
            request.algorithm = algorithm;

            response = call(client, request);
            success = response.success;

            assert(success, "Invalid displacement or rotation angle.");

            maximum_intensity = response.maximum_intensity;
        end

        function waveform = get_default_waveform(obj, channel)
            client = obj.get_default_waveform_client;

            request = ros2message(client);

            request.channel = int8(channel);

            response = call(client, request);
            success = response.success;

            assert(success, "Invalid channel.");

            waveform = response.waveform;
        end

        function waveform = reverse_polarity(obj, waveform)
            client = obj.reverse_polarity_client;

            request = ros2message(client);

            request.waveform = waveform;

            response = call(client, request);
            success = response.success;

            assert(success, "Failed request.");

            waveform = response.waveform;
        end

        function [initial_voltages, approximated_waveforms] = get_multipulse_waveforms(obj, targets, target_waveforms)
            client = obj.get_multipulse_waveforms_client;

            request = ros2message(client);

            request.targets = targets;

            for i = 1:length(target_waveforms)
                request.target_waveforms(i) = target_waveforms{i};
            end

            response = call(client, request);
            success = response.success;

            assert(success, "Invalid number of pulses, targets, or target waveforms.");

            initial_voltages = response.initial_voltages;
            approximated_waveforms = response.approximated_waveforms;
        end

        % Other

        function [mep, errors] = analyze_mep(obj, time, mep_configuration)
            client = obj.analyze_mep_client;

            request = ros2message(client);

            request.time = time;
            request.mep_configuration.emg_channel = uint8(mep_configuration.emg_channel);
            request.mep_configuration.time_window = mep_configuration.time_window;
            request.mep_configuration.preactivation_check = mep_configuration.preactivation_check;

            try
                response = call(client, request, 'timeout', 5);

                % If successful, assign the output from the response.
                mep = response.mep;
                errors = response.errors;

            catch ME
                warning('MEP analysis service call timed out or failed.');

                % Return zeros to indicate failed call.
                mep.amplitude = 0;
                mep.latency = 0;
                errors = "Unkown";

                % Re-initialize the client
                obj.analyze_mep_client = ros2svcclient(obj.node, "/mtms/mep/analyze_service", "mep_interfaces/AnalyzeMepService");

                % Give a moment for the new client to establish connection
                pause(0.5);
            end
        end

        % System state

        function handle_system_state(obj, system_state)
            obj.system_state = system_state;
        end

        function wait_for_new_state(obj)
            obj.system_state = NaN;
            while ~isstruct(obj.system_state)
                pause(0.1)
            end

            obj.printer.print_state(obj.system_state, obj.session);
        end

        % Decider
        function handle_decider_enabled(obj, msg)
            if msg.data
                delete(obj.node);

                % Disable API completely if Decider is enabled; this is to prevent Decider from inadvertently charging
                % the channels to the maximum voltage (due to using PWM) while API is being used to deliver pulses.
                assert(false, "Decider is enabled. All API operations are now disabled. Please restart.");
            end
        end

        % Session

        function handle_session(obj, session)
            obj.session = session;
        end
    end
end
