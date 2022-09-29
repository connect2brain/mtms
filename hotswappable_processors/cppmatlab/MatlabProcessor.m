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
        c3_file_id
        ampl_file_id
        phase_file_id
        estimated
        samples_collected
        print_c3
    end

    methods
        function obj = constructor(obj)            
            obj.A = 1;
            obj.HILBERTWIN = 64;
            obj.EDGE = 35;
            obj.AR_ORDER = 15;
            obj.FS = 5000;

            obj.offset_correction = 0;
            obj.nr_seconds = 1;
            obj.nr_samples = obj.nr_seconds * obj.FS;

            obj.lpf = firls(80, [0 80 250 5000/2]/(5000/2), [1 1 0 0], [1 1]);
            obj.bpf = firls(80, [0 6 9 13 16 (500/2)]/(500/2), [0 0 1 1 0 0], [1 1 1]);

            obj.set_channel_count(1);
            obj.set_window_size(obj.FS);
            obj.set_auto_enqueue(false);
            
            obj.estimated = false;
            obj.print_c3 = false;
            obj.samples_collected = 0;

            obj.c3_file_id = fopen('data.csv', 'w');
            obj.ampl_file_id = fopen('amplitudes.csv', 'w');
            obj.phase_file_id = fopen('phases.csv', 'w');

            fprintf(obj.c3_file_id, 'c3\n');
            fprintf(obj.ampl_file_id, 'estimated_amplitude\n');
            fprintf(obj.phase_file_id, 'estimated_phase\n');
        end
        function on_init_experiment(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time_us, first_sample_of_experiment)
            c3 = channel_data(5) - 0.25 * (channel_data(21) + channel_data(23) + channel_data(25) + channel_data(27));
            obj.enqueue(c3);
            obj.samples_collected = obj.samples_collected + 1;

            if ~obj.print_c3 && obj.estimated && obj.samples_collected > obj.FS + obj.FS / 2
                %fprintf(obj.c3_file_id, "%f\n", c3);
                data = obj.data(1:10:end);
                for i=1:numel(data)
                    fprintf(obj.c3_file_id, "%f\n", data(i));
                end
                obj.print_c3 = true;
            end
            
            if obj.samples_collected == obj.nr_samples && ~obj.estimated
                % fprintf(obj.file_id, "%f,0,0\n", c3);
                
                %downsampled = obj.data(1:10:end);

                data = filter(obj.lpf, obj.A, obj.data);
                data = data(1:10:end);

                [estimated_phases, estimated_amplitudes] = phastimate(data(1:250), obj.bpf, obj.EDGE, obj.AR_ORDER, obj.HILBERTWIN);
                s = size(estimated_phases);
                fprintf("num of estimated phases: %f, %f\n", s(1), s(2));
                s = size(estimated_amplitudes);
                fprintf("num of estimated amplitudes: %f, %f\n", s(1), s(2));
                
                
                fprintf("Estimation done\n");
                obj.estimated = true;

                for index=1:numel(estimated_amplitudes)
                    ampl = estimated_amplitudes(index);
                    phase = estimated_phases(index);
                    fprintf(obj.ampl_file_id, "%f\n", ampl);
                    fprintf(obj.phase_file_id, "%f\n", phase);
                end

            else
                if ~obj.estimated
                    fprintf("Samples collected %f / %f\n", obj.samples_collected, obj.nr_samples);
                    %fprintf(obj.ampl_file_id, '%f\n', 0);
                    %fprintf(obj.phase_file_id, '%f\n', 0);
                end
                obj.set_commands([]);
            end
            obj.set_commands([]);
            
        end
        function on_end_experiment(obj)
            obj.commands = [];
        end
    end
end

