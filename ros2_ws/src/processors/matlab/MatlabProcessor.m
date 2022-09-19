classdef MatlabProcessor < AbstractMatlabProcessor

    properties(Access=private)
        file_id
        peak_detection
        peak_over
    end

    methods
        function obj = constructor(obj)
            disp("in ConcreteMatlabProcessor constructor");

            obj.set_channel_count(63);
            obj.set_window_size(50);
            %obj.set_auto_enqueue(true);
        end
        function on_init_experiment(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time_us, first_sample_of_experiment)
            obj.commands = [];
        end
        function on_end_experiment(obj)
            obj.commands = [];
        end
    end
end

