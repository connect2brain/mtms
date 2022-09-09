classdef MatlabProcessorInterface < handle

    properties
        data
        window_size
        channel_count
        pulses
    end


    methods 
        function obj = MatlabProcessorInterface(window_size, channel_count)
            obj.data = zeros(channel_count, window_size);
            % obj.data = zeros(1, window_size);
            obj.window_size = window_size;
            obj.channel_count = channel_count;
            pulse = obj.create_pulse_command();
            %display(pulse);
            coder.cstructname(pulse, 'stimulation_pulse_event');
            coder.cstructname(pulse.event_info, 'event_info');
            coder.cstructname(pulse.pieces, 'stimulation_pulse_piece');
            
            number_of_pulses = 5;
            if number_of_pulses > window_size
                number_of_pulses = 20;
            end
            obj.pulses = repmat(pulse, number_of_pulses, 1);
        end
        function ret = init_experiment(obj)
            ret = [];
        end
        function ret = data_received(obj, channel_data, time_us, first_sample_of_experiment)
            obj.enqueue(channel_data);
            % a trick to allocate an array of structs
            pulse = obj.create_pulse_command();
            if sum(channel_data) > 1
                number_of_pulses = 5;
            else
                number_of_pulses = 0;
            end
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