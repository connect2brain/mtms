classdef rotating_buffer < handle
    %CIRCULAR_BUFFER Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        window_size
        columns
        index
        full
        buffer
        nof_elements
    end
    
    methods
        function obj = rotating_buffer(window_size, columns)
            obj.window_size = uint32(window_size);
            obj.columns = uint32(columns);
            obj.full = false;
            obj.index = uint32(1);
            obj.buffer = zeros(window_size, columns);

            obj.nof_elements = 0;
        end
        
        function append(obj, sample)
            obj.nof_elements = obj.nof_elements + 1;
            if obj.nof_elements == obj.window_size
                obj.full = true;
            end

            if ~obj.full
                obj.buffer(obj.index, :) = sample;
            else
                obj.buffer = circshift(obj.buffer, -1);
                obj.buffer(end, :) = sample;
            end
        end

        function buffer = get_buffer(obj)
            buffer = obj.buffer;
        end

        function obj = set_buffer(obj, buffer)
            obj.buffer = buffer;
        end

        function sample = at(obj, index)
            sample = obj.buffer(index);
        end

        function print(obj)
            buf = obj.get_buffer();
            row_limit = min(obj.window_size, obj.nof_elements);
            for row=1:row_limit
                fprintf("[");
                for col=1:obj.columns
                    fprintf("%f ", buf(row, col));
                end
                fprintf("]\n");
            end
        end
    end
end

