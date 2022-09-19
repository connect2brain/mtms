classdef (Abstract) AbstractMatlabProcessor < handle
    %ABSTRACTMATLABPROCESSOR Summary of this class goes here
    %   Detailed explanation goes here

    properties
        data
        window_size
        channel_count
        commands
        events_sent
        experiment_started
        last_sample_received_at_us
    end

    methods (Abstract)
        constructor(obj, arg1, arg2)
        on_data_received(obj, channel_data, time_us, first_sample_of_experiment)
        on_init_experiment(obj)
        on_end_experiment(obj)
    end

    methods
        function obj = AbstractMatlabProcessor()
            %ABSTRACTMATLABPROCESSOR Construct an instance of this class
            %   Detailed explanation goes here
            disp("in abastract constructor");

            obj.window_size = uint16(50);
            obj.channel_count = uint16(62);
            obj.data = zeros(obj.channel_count, obj.window_size);
            obj.experiment_started = false;
            obj.last_sample_received_at_us = uint64(0);

            % Tells the compiler to make commands a variable sized list
            number_of_pulses = 5;
            if number_of_pulses > obj.window_size
                number_of_pulses = 20;
            end
            obj.commands = repmat(create_command(0, "pulse_event", 0), number_of_pulses, 1);

            obj.events_sent = uint32(0);
            
            obj.constructor();

        end

        function ret = data_received(obj, channel_data, time_us, first_sample_of_experiment)
            if first_sample_of_experiment
                obj.experiment_started = true;
            end
            obj.on_data_received(channel_data, time_us, first_sample_of_experiment);
            obj.events_sent = obj.events_sent + size(obj.commands, 2);
            obj.last_sample_received_at_us = time_us;
            
            ret = obj.commands;
        end
        function ret = init_experiment(obj)
            obj.on_init_experiment();
            ret = obj.commands;
        end
        function ret = end_experiment(obj)
            obj.on_end_experiment();
            ret = obj.commands;
        end

        function obj = set_window_size(obj, new_window_size)
            %set_window_size Set window size
            %   Resets data to zeros as its dimensions change
            obj.window_size = uint16(new_window_size);
            obj.data = zeros(obj.channel_count, obj.window_size);
        end
        function obj = set_channel_count(obj, new_channel_count)
            %set_channel_count Set channel count
            %   Resets data to zeros as its dimensions change
            obj.channel_count = uint16(new_channel_count);
            obj.data = zeros(obj.channel_count, obj.window_size);
        end
        function val = getData(obj)
            val = obj.data;
        end
        function obj = setData(obj, val)
            obj.data = val;
        end
        function ret = enqueue(obj, element)
            temp = obj.data(1);
            for i = 1:obj.window_size - 1
                obj.data(i) = obj.data(i + 1);
            end
            obj.data(:,end) = element;
            ret = temp;
        end
    end
end

