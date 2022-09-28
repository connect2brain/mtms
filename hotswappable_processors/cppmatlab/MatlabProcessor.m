classdef MatlabProcessor < AbstractMatlabProcessor
    
    properties(Access=private)
        A
        HILBERTWIN
        EDGE
        AR_ORDER
        FS

        offset_correction

        nr_seconds
        nr_samples

        lpf
        bpf
        file_id
     
        
    end

    methods
        function obj = constructor(obj)            
            obj.A = 1;
            obj.HILBERTWIN = 64;
            obj.EDGE = 35;
            obj.AR_ORDER = 15;
            obj.FS = 500;

            obj.offset_correction = 0;
            obj.nr_seconds = 1;
            obj.nr_samples = obj.nr_seconds * obj.FS;

            obj.lpf = firls(80, [0 80 250 5000/2]/(5000/2), [1 1 0 0], [1 1]);
            obj.bpf = firls(80, [0 6 9 13 16 (500/2)]/(500/2), [0 0 1 1 0 0], [1 1 1]);

            obj.set_channel_count(1);
            obj.set_window_size(obj.FS);
            obj.set_auto_enqueue(false);
            
            obj.file_id = fopen('data.csv', 'w');
            fprintf(obj.file_id, 'c3,estimated_amplitude,estimated_phase\n');
        end
        function on_init_experiment(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time_us, first_sample_of_experiment)
            c3 = channel_data(5) - 0.25 * (channel_data(21) + channel_data(23) + channel_data(25) + channel_data(27));
            obj.enqueue(c3);
            %fprintf("numel obj.data: %f\n", numel(obj.data));
            %fprintf("obj.nr_samples: %f\n", obj.nr_samples);
            if numel(obj.data) == obj.nr_samples
                fprintf(obj.file_id, "%f,0,0\n", c3);
                
                %downsampled = obj.data(1:10:end);

                data = filter(obj.lpf, obj.A, obj.data);
                data = data(1:10:end);

                [estimated_phases, estimated_amplitudes] = phastimate(data(1:250, :), obj.bpf, obj.EDGE, obj.AR_ORDER, obj.HILBERTWIN);
                fprintf("num of estimated phases: %f\n", numel(estimated_phases));
                fprintf("num of estimated amplitudes: %f\n", numel(estimated_amplitudes));

                for i = 1:numel(estimated_phases)
                    fprintf("%f, ", estimated_phases(i));
                end
                fprintf("\n");
                
                pulse = create_command(obj.events_sent + 1, "pulse_event", 0);
                charge = create_command(obj.events_sent + 2, "charge_event", 1200);
                obj.set_commands([pulse, charge]);
            else
                obj.set_commands([]);
            end
            
        end
        function on_end_experiment(obj)
            obj.commands = [];
        end
    end
end

