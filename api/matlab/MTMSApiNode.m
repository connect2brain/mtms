classdef MTMSApiNode < handle
    %MTMSApiNode A class for communicating with other ROS nodes.
    
    properties
        node
        printer
        system_state

        event_feedback

        start_device_client
        stop_device_client
        start_experiment_client
        stop_experiment_client

        send_charge_publisher
        send_discharge_publisher
        send_trigger_out_publisher
        send_pulse_publisher

        send_event_trigger_publisher

        system_state_subscriber 

        pulse_feedback_subscriber
        trigger_out_feedback_subscriber
        charge_feedback_subscriber
        discharge_feedback_subscriber

        get_channel_voltages_client
        get_maximum_intensity_client
        get_default_waveform_client
        reverse_polarity_client

        analyze_mep_client
    end
    
    methods
        function obj = MTMSApiNode()

            % Currently in the Aalto mTMS lab, the API seems to work without changing
            % RMW_IMPLEMENTATION. Hence, comment out the line below. If in the
            % future it is required (e.g., due to updating to a more recent ROS
            % version), the line can be uncommented.
            %

            % setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp")

            obj.node = ros2node("mtms_matlab_api");
            obj.printer = MTMSApiPrinter();

            obj.system_state = NaN;
            obj.event_feedback = dictionary();

            % To mTMS device.

            obj.start_device_client = ros2svcclient(obj.node, "/mtms_device/start_device", "mtms_device_interfaces/StartDevice");
            obj.stop_device_client = ros2svcclient(obj.node, "/mtms_device/stop_device", "mtms_device_interfaces/StopDevice");

            obj.start_experiment_client = ros2svcclient(obj.node, "/mtms_device/start_experiment", "mtms_device_interfaces/StartExperiment");
            obj.stop_experiment_client = ros2svcclient(obj.node, "/mtms_device/stop_experiment", "mtms_device_interfaces/StopExperiment");

            obj.system_state_subscriber = ros2subscriber(obj.node, "/mtms_device/system_state", "mtms_device_interfaces/SystemState", @obj.handle_system_state);

            % Event-related.

            obj.send_charge_publisher = ros2publisher(obj.node, "/event/send/charge", "event_interfaces/Charge");
            obj.send_discharge_publisher = ros2publisher(obj.node, "/event/send/discharge", "event_interfaces/Discharge");
            obj.send_trigger_out_publisher = ros2publisher(obj.node, "/event/send/trigger_out", "event_interfaces/TriggerOut");
            obj.send_pulse_publisher = ros2publisher(obj.node, "/event/send/pulse", "event_interfaces/Pulse");

            obj.send_event_trigger_publisher = ros2publisher(obj.node, "/event/send/event_trigger", "event_interfaces/EventTrigger");

            obj.pulse_feedback_subscriber = ros2subscriber(obj.node, "/event/pulse_feedback", "event_interfaces/PulseFeedback", @obj.handle_pulse_feedback);
            obj.charge_feedback_subscriber = ros2subscriber(obj.node, "/event/charge_feedback", "event_interfaces/ChargeFeedback", @obj.handle_charge_feedback);
            obj.discharge_feedback_subscriber = ros2subscriber(obj.node, "/event/discharge_feedback", "event_interfaces/DischargeFeedback", @obj.handle_discharge_feedback);
            obj.trigger_out_feedback_subscriber = ros2subscriber(obj.node, "/event/trigger_out_feedback", "event_interfaces/TriggerOutFeedback", @obj.handle_trigger_out_feedback);

            % To other parts of the system.

            obj.get_channel_voltages_client = ros2svcclient(obj.node, "/targeting/get_channel_voltages", "targeting_interfaces/GetChannelVoltages");
            obj.get_maximum_intensity_client = ros2svcclient(obj.node, "/targeting/get_maximum_intensity", "targeting_interfaces/GetMaximumIntensity");
            obj.get_default_waveform_client = ros2svcclient(obj.node, "/waveforms/get_default", "targeting_interfaces/GetDefaultWaveform");
            obj.reverse_polarity_client = ros2svcclient(obj.node, "/waveforms/reverse_polarity", "targeting_interfaces/ReversePolarity");

            obj.analyze_mep_client = ros2svcclient(obj.node, "/mep/analyze_service", "mep_interfaces/AnalyzeMepService");
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

        function success = start_experiment(obj)
            %start_experiment Start experiment on mTMS device
            %
            %   Start an experiment on the mTMS device.

            client = obj.start_experiment_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        function success = stop_experiment(obj)
            %stop_experiment Stop experiment on mTMS device
            %
            %   Stop an experiment on the mTMS device.

            client = obj.stop_experiment_client;

            request = ros2message(client);

            response = call(client, request);
            success = response.success;
        end

        % Events

        function send_event_trigger(obj)
            %send_event_trigger Send event trigger
            %
            %   Send event trigger to mTMS device.

            publisher = obj.send_event_trigger_publisher;

            event_trigger = ros2message(publisher);

            send(publisher, event_trigger);
        end

        function send_pulse(obj, id, execution_condition, time, channel, waveform)
            %send_pulse Send pulse event
            %
            %   Send pulse event to the mTMS device.

            publisher = obj.send_pulse_publisher;

            pulse = ros2message(publisher);

            event_info = ros2message("event_interfaces/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            pulse.event_info = event_info;
            pulse.channel = uint8(channel);
            pulse.waveform = waveform;

            send(publisher, pulse);
        end

        function send_charge(obj, id, execution_condition, time, channel, target_voltage)
            %send_charge Send charge event
            %
            %   Sends charge event to the mTMS device.

            publisher = obj.send_charge_publisher;

            charge = ros2message(publisher);
            
            event_info = ros2message("event_interfaces/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            charge.event_info = event_info;
            charge.channel = uint8(channel);
            charge.target_voltage = uint16(target_voltage);

            send(publisher, charge);
        end

        function send_discharge(obj, id, execution_condition, time, channel, target_voltage)
            %send_discharge Send discharge command to mTMS device
            %
            %   Sends discharge command to the mTMS device.

            publisher = obj.send_discharge_publisher;

            discharge = ros2message(publisher);

            event_info = ros2message("event_interfaces/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);

            discharge.event_info = event_info;
            discharge.channel = uint8(channel);
            discharge.target_voltage = uint16(target_voltage);

            send(publisher, discharge);
        end

        function send_trigger_out(obj, id, execution_condition, time, port, duration_us)
            %send_trigger_out Send trigger out event
            %
            %   Send trigger out event to the mTMS device.

            publisher = obj.send_trigger_out_publisher;

            trigger_out = ros2message(publisher);

            event_info = ros2message("event_interfaces/EventInfo");
            event_info.id = uint16(id);
            event_info.execution_condition.value = execution_condition;
            event_info.execution_time = double(time);
            
            trigger_out.event_info = event_info;
            trigger_out.port = uint8(port);
            trigger_out.duration_us = uint32(duration_us);

            send(publisher, trigger_out);
        end

        % Feedback

        function update_event_feedback(obj, feedback)
            id = feedback.id;
            error = feedback.error;
    
            obj.event_feedback(id) = error;
        end

        function feedback = get_event_feedback(obj, id)
            feedback = NaN;
            if ~isKey(obj.event_feedback, id)
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
    
        function [voltages, reverse_polarities] = get_channel_voltages(obj, displacement_x, displacement_y, rotation_angle, intensity)

            client = obj.get_channel_voltages_client;

            request = ros2message(client);

            request.displacement_x = int8(displacement_x);
            request.displacement_y = int8(displacement_y);
            request.rotation_angle = uint16(rotation_angle);
            request.intensity = uint8(intensity);

            response = call(client, request);
            success = response.success;

            assert(success, "Invalid displacement, rotation angle, or intensity.");
    
            voltages = response.voltages;
            reverse_polarities = response.reversed_polarities;
        end

        function maximum_intensity = get_maximum_intensity(obj, displacement_x, displacement_y, rotation_angle)

            client = obj.get_maximum_intensity_client;

            request = ros2message(client);

            request.displacement_x = int8(displacement_x);
            request.displacement_y = int8(displacement_y);
            request.rotation_angle = uint16(rotation_angle);

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

        % Other

        function [amplitude, latency, errors] = analyze_mep(obj, emg_channel, time, mep_configuration)
            client = obj.analyze_mep_client;

            request = ros2message(client);
    
            request.emg_channel = uint8(emg_channel);
            request.time = time;
            request.mep_configuration = mep_configuration;

            response = call(client, request);

            amplitude = response.amplitude;
            latency = response.latency;
            errors = response.errors;
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

            obj.printer.print_system_state(obj.system_state);
        end
    end
end
