classdef ring_buffer < handle
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
        function obj = ring_buffer(window_size, columns)
            obj.window_size = uint32(window_size);
            obj.columns = uint32(columns);
            obj.full = false;
            obj.index = uint32(1);
            obj.buffer = zeros(window_size, columns);

            obj.nof_elements = 0;
        end
        
        function append(obj, sample)
            obj.buffer(obj.index, :) = sample;
            obj.index = obj.index + 1;

            if obj.index > obj.window_size
                obj.index = obj.index - obj.window_size;
            end

            if obj.index == 1
                obj.full = true;
            else
                obj.nof_elements = obj.nof_elements + 1;
            end
        end

        function buffer = get_buffer(obj)
            if ~obj.full
                buffer = obj.buffer(1:obj.index, :);
            else
                left = obj.buffer(obj.index:end, :);
                right = obj.buffer(1:obj.index - 1, :);
                buffer = cat(1, left, right);
            end
        end

        function obj = set_buffer(obj, buffer)
            obj.buffer = buffer;
        end

        function sample = at(obj, index)
            sample = obj.buffer(mod(index + obj.index, obj.window_size));
        end

        function print(obj)
            buf = obj.get_buffer();
            row_limit = min(obj.window_size, obj.nof_elements);
            for row=1:row_limit
                fprintf("[");
                for col=1:obj.columns
                    fprintf("%f ", buf(row, col));
                end
                fprintf("]");
                fprintf(" | ");
                fprintf("[");
                for col=1:obj.columns
                    fprintf("%f ", obj.buffer(row, col));
                end
                fprintf("]\n");
            end
        end
    end
end

