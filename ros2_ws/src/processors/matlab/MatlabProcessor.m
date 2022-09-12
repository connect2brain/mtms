classdef MatlabProcessor < handle

    properties
        data
        window_size
        channel_count
        pulses
        peak_detection
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
            pulse = obj.create_pulse_command();
            
            number_of_pulses = 5;
            if number_of_pulses > window_size
                number_of_pulses = 20;
            end
            obj.pulses = repmat(pulse, number_of_pulses, 1);
            obj.peak_detection = Thresholding(zeros(window_size, 1), window_size, 5.5, 1);
            obj.file_id = fopen("eeg_data.csv", "w");
            fprintf(obj.file_id, "c3,filtered,spike\n");
        end
        function ret = init_experiment(obj)
            ret = [];
        end
        function close(obj)
            fclose(obj.file_id);
        end
        function ret = data_received(obj, channel_data, time_us, first_sample_of_experiment)
            obj.enqueue(channel_data);
            c3 = channel_data(5);
            fprintf("length %f\n", obj.peak_detection.length);

            signal = obj.peak_detection.thresholding_algo(c3);
            pulse = obj.create_pulse_command();
            spike_mark = "f";
            if signal == 1
                number_of_pulses = 1;
                spike_mark = "t";
            else
                number_of_pulses = 0;
            end
            if signal ~= 0
                fprintf("%f\n", signal);
            end

            fprintf(obj.file_id, "%6.2f, %f, %s\n", c3, 1, spike_mark);
            obj.pulses = repmat(pulse, number_of_pulses, 1);
            ret = obj.pulses;
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
        function command = create_pulse_command(obj)
            % Create pulse command
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
            
            pulse_command.channel = uint8(5);
            pulse_command.event_info = event_info;
            pulse_command.pieces = pieces;
            
            coder.cstructname(pulse_command, 'stimulation_pulse_event');
            coder.cstructname(pulse_command.event_info, 'event_info');
            coder.cstructname(pulse_command.pieces, 'stimulation_pulse_piece');

            command = pulse_command;
        end
    end
end