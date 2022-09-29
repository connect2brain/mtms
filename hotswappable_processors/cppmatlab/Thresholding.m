% Adapated from Brakel, J.P.G. van (2014). "Robust peak detection algorithm using z-scores". Stack Overflow. Available at: https://stackoverflow.com/questions/22583391/peak-signal-detection-in-realtime-timeseries-data/22640362#22640362 (version: 2020-11-08).

classdef Thresholding < handle
    %THERSHOLDING Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        signals
        avgFilter
        stdFilter
        filteredY
        length
        y
        lag
        threshold
        influence
        max_len
    end
    
    methods
        function obj = Thresholding(array, lag, threshold, influence)
            % coder.inline("always");
            y = array;

            obj.y = y;
            obj.length = length(obj.y);
            obj.lag = lag;
            obj.threshold = threshold;
            obj.influence = influence;

            obj.filteredY = y;
            obj.signals = zeros(obj.length,1);
            obj.avgFilter = zeros(obj.length, 1);
            obj.stdFilter = zeros(obj.length, 1);
            obj.avgFilter(lag) = mean(obj.y(1:lag));
            obj.stdFilter(lag) = std(obj.y(1:lag));
        end
        
        function value = enqueue_y(obj, new_value)
            ret = obj.y(1);
            for i=1:obj.length-1
                obj.y(i) = obj.y(i + 1);
            end
            obj.y(end) = new_value;
            value = ret;
        end

        function value = enqueue_signals(obj, new_value)
            ret = obj.signals(1);
            for i=1:obj.length-1
                obj.signals(i) = obj.signals(i + 1);
            end
            obj.signals(end) = new_value;
            value = ret;
        end

        function value = enqueue_filtered_y(obj, new_value)
            ret = obj.filteredY(1);
            for i=1:obj.length-1
                obj.filteredY(i) = obj.filteredY(i + 1);
            end
            obj.filteredY(end) = new_value;
            value = ret;
        end

        function value = enqueue_avg_filter(obj, new_value)
            ret = obj.avgFilter(1);
            for i=1:obj.length-1
                obj.avgFilter(i) = obj.avgFilter(i + 1);
            end
            obj.avgFilter(end) = new_value;
            value = ret;
        end

        function value = enqueue_std_filter(obj, new_value)
            ret = obj.stdFilter(1);
            for i=1:obj.length-1
                obj.stdFilter(i) = obj.stdFilter(i + 1);
            end
            obj.stdFilter(end) = new_value;
            value = ret;
        end


        function signal = thresholding_algo(obj, new_value)
            % coder.inline("always");
            % obj.y(end + 1) = new_value;
            obj.enqueue_y(new_value);
            obj.enqueue_signals(0);
            obj.enqueue_filtered_y(0);
            
            disp(size(obj.filteredY));

            obj.enqueue_avg_filter(0);
            obj.enqueue_std_filter(0);
            i = obj.length;
            
            %fprintf("%f\n", new_value);

            if abs(obj.y(i) - obj.avgFilter(i - 1)) > (obj.threshold * obj.stdFilter(i - 1))
                if obj.y(i) > obj.avgFilter(i - 1)
                    obj.signals(i) = 1;
                else
                    obj.signals(i) = -1;
                end

                obj.filteredY(i) = obj.influence * obj.y(i) + (1 - obj.influence) * obj.filteredY(i - 1);
                obj.avgFilter(i) = mean(obj.filteredY((i - obj.lag):i));
                obj.stdFilter(i) = std(obj.filteredY((i - obj.lag):i));
            else
                obj.signals(i) = 0;
                obj.filteredY = obj.y(i, :);
                d = obj.filteredY((i - obj.lag) : i);
                obj.avgFilter(i) = mean(d);
                obj.stdFilter(i) = std(d);
            end
            % fprintf("y %f, signals %f, filteredY %f, avgFilter %f, stdFilter %f\n", obj.y(end), obj.signals(end), obj.filteredY(end), obj.avgFilter(end), obj.stdFilter(end));
            signal = obj.signals(i);

        end
    end
end

