classdef MatlabProcessor < handle

    properties
        data
        window_size
        channel_count
        commands
        peak_detection
        spike_over
    end
    properties(Access=private)
        file_id
    end

    methods 
        function obj = MatlabProcessor(window_size, channel_count)
            obj.data = zeros(channel_count, window_size);
            % obj.data = zeros(1, window_size);
            obj.window_size = window_size;
            obj.channel_count = channel_count;
            pulse = obj.create_command("pulse_event", 0);
            
            number_of_pulses = 5;
            if number_of_pulses > window_size
                number_of_pulses = 20;
            end
            obj.commands = repmat(pulse, number_of_pulses, 1);

            obj.peak_detection = Thresholding(zeros(window_size, 1), 30, 5.5, 0.7);
            obj.file_id = fopen("eeg_data.csv", "w");
            fprintf(obj.file_id, "c3,filtered,spike\n");
            obj.spike_over = true;
            fprintf("matlab processor init\n");
        end
        function ret = init_experiment(obj)
            ret = [];
        end
        function close(obj)
            fclose(obj.file_id);
        end
        function ret = data_received(obj, channel_data, time_us, first_sample_of_experiment)
            fprintf("matlab data received \n");
            fprintf("%f\n", channel_data(1));
            fprintf("%d\n", int32(obj.window_size));
            obj.enqueue(channel_data);
            c3 = channel_data(5);
            fprintf("c3: %f\n", c3);

            signal = obj.peak_detection.thresholding_algo(c3);
            if signal == 0 && ~obj.spike_over
                obj.spike_over = true;
            end
            
            pulse = obj.create_command("pulse_event", 0);
            % charge = obj.create_command("charge_event", 50);
            spike_mark = "f";

            if signal == 1 && obj.spike_over
                number_of_pulses = 1;
                spike_mark = "t";
                obj.spike_over = false;
            else
                number_of_pulses = 0;
            end
            
            fprintf("%s\n", spike_mark);
            fprintf(obj.file_id, "%6.2f,%f,%s\n", c3, 1, spike_mark);

            % number_of_pulses = 2;

            obj.commands = repmat(pulse, number_of_pulses, 1);
            % obj.commands(1) = charge;
            ret = obj.commands;
        end
    end
    methods(Access=private)
        function ret = enqueue(obj, element)
            coder.inline("always");
            temp = obj.data(1);
            for i = 1:obj.window_size - 1
                obj.data(i) = obj.data(i + 1);
            end
            obj.data(:,end) = element;
            ret = temp;
        end
        function command = create_command(obj, event_type, target_voltage)
            event_info.event_id = uint16(0);
            event_info.execution_condition = uint8(0);
            event_info.time_us = uint64(0);
            
            piece1.mode = uint8(0);
            piece1.duration_in_ticks = uint16(200);
            piece2.mode = uint8(2);
            piece2.duration_in_ticks = uint16(160);
            piece3.mode = uint8(1);
            piece3.duration_in_ticks = uint16(160);
            
            pieces = [piece1, piece2, piece3];
            
            command.channel = uint8(5);
            command.event_info = event_info;
            command.pieces = pieces;
            if event_type == "pulse_event"
                command.event_type = uint8(0);
            elseif event_type == "charge_event"
                command.event_type = uint8(1);
            else
                command.event_type = uint8(2);
            end

            command.target_voltage = uint16(target_voltage);

            coder.cstructname(command, 'matlab_fpga_event');
            coder.cstructname(command.event_info, 'event_info');
            coder.cstructname(command.pieces, 'stimulation_pulse_piece');
        end
    end
end