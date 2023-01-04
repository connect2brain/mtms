classdef MatlabProcessor < AbstractMatlabProcessor
    
    properties(Access=private)
        sampling_frequency

        samples_collected

        isi_samples
        isi_seconds

        full
    end

    methods
        function obj = constructor(obj)     
            obj.sampling_frequency = 5000;

            obj.set_channel_count(62);
            obj.set_window_size(obj.sampling_frequency);

            obj.set_auto_enqueue(true);
            
            obj.samples_collected = 0;
            obj.full = false;

            obj.isi_seconds = 1;
            obj.isi_samples = 100;%obj.sampling_frequency * obj.isi_seconds;
            
        end
        function on_init_experiment(obj)
            obj.set_commands([]);
        end
        function on_data_received(obj, channel_data, time, first_sample_of_experiment)
            obj.samples_collected = obj.samples_collected + 1;
            if obj.samples_collected == obj.sampling_frequency
                obj.full = true;
            end

            if mod(obj.samples_collected, obj.isi_samples) == 0 && obj.full
                data = obj.get_data();
                event = create_signal_out_command(1, 1, 1000, 2, time);
                obj.samples_collected = 0;
                
                obj.set_commands([event]);


            else
                obj.set_commands([]);
            end
            
        end
        function on_end_experiment(obj)
            obj.set_commands([]);
        end
    end
end

