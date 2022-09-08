classdef Processor < handle

    properties
        data
        window_size
    end

    methods 
        function obj = Processor(window_size)
            obj.data = zeros(62, window_size);
            % obj.data = zeros(1, window_size);
            obj.window_size = window_size;
        end
        function ret = init_experiment(obj)
            ret = [];
        end
        function ret = data_received(obj, channel_data, time_us, first_sample_of_experiment)

            obj.enqueue(channel_data);
            ret = [];
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