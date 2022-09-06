classdef Processor < handle

    properties
        data
    end

    methods 
        function ret = add(obj, element)
            obj.data(end + 1) = element;
            ret = size(obj.data);
        end

        function ret = add_simple(obj)
            obj.data(end + 1) = 3;
            ret = size(obj.data);
        end
    end
end