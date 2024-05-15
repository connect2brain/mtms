classdef WaveformApproximatorWrapper < handle
    %WaveformApproximator A wrapper class for approximating waveforms.

    properties
        node
        target
        approximator

        approximate_waveform_server
        estimate_voltage_after_pulse_server
    end

    methods
        function obj = WaveformApproximatorWrapper()
            setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp")

            obj.node = ros2node("waveform_approximator");

            % Create ROS server
            obj.approximate_waveform_server = ros2svcserver(obj.node, "/targeting/approximate_waveform", ...
                "targeting_interfaces/ApproximateWaveform", ...
                @obj.approximate_waveform);

            obj.estimate_voltage_after_pulse_server = ros2svcserver(obj.node, "/targeting/estimate_voltage_after_pulse", ...
                "targeting_interfaces/EstimateVoltageAfterPulse", ...
                @obj.estimate_voltage_after_pulse);

            resolution = 0.01e-6;
            solutions_file = 'solutions_five_coil_set.mat';

            obj.approximator = WaveformApproximator(solutions_file, resolution);
        end

        % Convert the ROS message to a MATLAB struct
        function waveform_struct = convert_ros_waveform_to_matlab_waveform(obj, waveform)
            pieces = waveform.pieces;

            % Initialize the mode string and durations array
            modes = '';
            durations_in_ticks = zeros(1, length(pieces));

            for i = 1:length(pieces)
                piece = pieces(i);
                phase = piece.waveform_phase;

                % Map the waveform phase in ROS message to a character
                switch phase.value
                    case phase.RISING
                        mode = 'r';
                    case phase.HOLD
                        mode = 'h';
                    case phase.FALLING
                        mode = 'f';
                    case phase.ALTERNATIVE_HOLD
                        mode = 'a';
                    otherwise
                        assert (false, 'Invalid phase');
                end

                % Concatenate the phase to the mode string
                modes = [modes mode];
                durations_in_ticks(i) = piece.duration_in_ticks;
            end

            % Convert ticks to seconds (1 tick = 25 ns)
            durations = double(durations_in_ticks) * 25 / 1e9;

            % Create waveform struct
            waveform_struct = struct();
            waveform_struct.modes = modes;
            waveform_struct.durations = durations;
        end

        % Convert the MATLAB struct to a ROS message
        function waveform = convert_matlab_waveform_to_ros_waveform(obj, waveform_struct)
            modes = waveform_struct.modes;

            % Create the waveform ROS message
            waveform = ros2message("event_interfaces/Waveform");

            % Loop through the modes and durations
            waveform.pieces = [];
            for i = 1:length(modes)
                piece = ros2message("event_interfaces/WaveformPiece");
                phase = piece.waveform_phase;

                % Map the character to the waveform phase in ROS message
                switch modes(i)
                    case 'r'
                        value = phase.RISING;
                    case 'h'
                        value = phase.HOLD;
                    case 'f'
                        value = phase.FALLING;
                    case 'a'
                        value = phase.ALTERNATIVE_HOLD;
                    otherwise
                        assert (false, 'Invalid phase');
                end
                piece.waveform_phase.value = value;

                % Convert seconds to ticks (1 tick = 25 ns)
                piece.duration_in_ticks = uint16(waveform_struct.durations(i) / 25 * 1e9);

                % Append the piece to the waveform ROS message
                waveform.pieces = [waveform.pieces piece];
            end
        end

        function response = approximate_waveform(obj, request, response)
            actual_voltage = double(request.actual_voltage);
            target_voltage = double(request.target_voltage);
            target_waveform = request.target_waveform;
            coil_number = request.coil_number;

            order_of_algorithms = {
                'constant_current', ...
                'constant_voltage', ...
                'linear_ramp', ...
                'exponential_ramp', ...
                'sine_wave', ...
                'square_wave', ...
                'triangle_wave', ...
                'sawtooth_wave', ...
                'custom_wave'
            }

            % Convert the ROS message to a MATLAB struct
            target_waveform = obj.convert_ros_waveform_to_matlab_waveform(target_waveform);

            % Select the coil
            obj.approximator.select_coil(coil_number);

            actual_voltage
            target_voltage
            target_waveform
            % Approximate the waveform
            while not hyväksyttävä vaihe:
                [success, approximated_waveform, ~] = obj.approximator.approximate(actual_voltage, target_voltage, target_waveform);
                
                if liian pitkä vaihe:
                    tee uusi funktio, joka ottaa parametrit ja palauttaa approximated_waveformin
                end
            end
            
            print("mitä tapahtuu tässä nyt")

            % Populate the response message
            if success
                response.approximated_waveform = obj.convert_matlab_waveform_to_ros_waveform(approximated_waveform);
            end
            response.success = success;
        end

        function response = estimate_voltage_after_pulse(obj, request, response)
            voltage_before = double(request.voltage_before);
            waveform = request.waveform;
            coil_number = request.coil_number;

            % Convert the ROS message to a MATLAB struct
            target_waveform = obj.convert_ros_waveform_to_matlab_waveform(waveform);

            % Select the coil
            obj.approximator.select_coil(coil_number);

            % Calculate the voltage after pulse for both waveforms
            voltage_after = obj.approximator.estimate_voltage_after_pulse(voltage_before, target_waveform);

            % Populate the response message
            response.voltage_after = uint16(voltage_after);
            response.success = true;
        end
    end
end
