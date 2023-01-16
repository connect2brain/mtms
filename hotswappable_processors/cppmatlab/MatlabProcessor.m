classdef MatlabProcessor < AbstractMatlabProcessor

    methods
        function obj = constructor(obj)            
            obj.set_channel_count(1);
            obj.set_window_size(1);
            obj.set_auto_enqueue(false);
        end
        function on_init_experiment(obj)
        end
        function on_data_received(obj, channel_data, time, first_sample_of_experiment)         
            sample = create_eeg_sample(channel_data', time, first_sample_of_experiment);
            obj.set_eeg_samples([sample]);
        end
        function on_end_experiment(obj)
        end
    end
end

