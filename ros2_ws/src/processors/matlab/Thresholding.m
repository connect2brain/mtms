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
    end
    
    methods
        function obj = Thresholding(array, lag, threshold, influence)
            coder.inline("always");
            obj.y = array;
            obj.lag = lag;
            obj.threshold = threshold;
            obj.influence = influence;

            obj.signals = zeros(length(obj.y),1);
            % Initialise filtered series
            obj.filteredY = obj.y(1:lag);
            % Initialise filters
            obj.avgFilter = zeros(length(obj.y), 1);
            obj.stdFilter = zeros(length(obj.y), 1);
            obj.avgFilter(lag) = mean(obj.y(1:lag));
            obj.stdFilter(lag) = std(obj.y(1:lag));
        end
        
        function signal = thresholding_algo(obj, new_value)
            coder.inline("always");
            %METHOD1 Summary of this method goes here
            %   Detailed explanation goes here
            obj.y(end + 1) = new_value;
            i = length(obj.y);
            obj.length = length(obj.y);

            if i < obj.lag
                signal = 0;
                return;
            end
            
            if i == obj.lag
                obj.signals = zeros(obj.length, 1);
                obj.filteredY = obj.y;
                obj.avgFilter(obj.lag+1) = mean(obj.y(1:obj.lag+1));
                obj.stdFilter(obj.lag+1) = std(obj.y(1:obj.lag+1));
                signal = 0;
                return;
            end

            obj.signals(end + 1) = 0;
            obj.filteredY(end +1) = 0;
            obj.avgFilter(end +1) = 0;
            obj.stdFilter(end +1) = 0;

            if abs(obj.y(i) - obj.avgFilter(i - 1)) > (obj.threshold * obj.stdFilter(i - 1))
                if obj.y(i - 1) > obj.avgFilter(i - 1 - 1)
                    obj.signals(i) = 1;
                else
                    obj.signals(i) = -1;
                end

                obj.filteredY(i) = obj.influence * obj.y(i) + (1 - obj.influence) * obj.filteredY(i - 1);
                obj.avgFilter(i) = mean(obj.filteredY((i - obj.lag):i));
                obj.stdFilter(i) = std(obj.filteredY((i - obj.lag):i));
            else
                obj.signals(i) = 0;
                obj.avgFilter(i) = mean(obj.filteredY((i - obj.lag):i));
                obj.stdFilter(i) = std(obj.filteredY((i - obj.lag):i));
            end

            signal = obj.signals(i);

        end
    end
end

