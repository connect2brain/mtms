classdef MatlabProcessor < AbstractMatlabProcessor

    methods
        function obj = constructor(obj)
            obj.set_channel_count(63);
            obj.set_window_size(50);
            obj.set_auto_enqueue(true);
        end
        function on_init_session(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time, first_sample_of_session)
            obj.set_commands([]);
        end
        function on_end_session(obj)
            obj.commands = [];
        end
    end
end

