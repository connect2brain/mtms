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
        lpf_file_id

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

            obj.offset_correction = obj.FS * 0.008;
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
            obj.lpf_file_id = fopen('lpf.csv', 'w');

            fprintf(obj.c3_file_id, 'c3\n');
            fprintf(obj.ampl_file_id, 'estimated_amplitude\n');
            fprintf(obj.phase_file_id, 'estimated_phase\n');
            fprintf(obj.lpf_file_id, 'lpf\n');
        end
        function on_init_experiment(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time_us, first_sample_of_experiment)
            c3 = channel_data(5) - 0.25 * (channel_data(21) + channel_data(23) + channel_data(25) + channel_data(27));
            obj.enqueue(c3);
            obj.samples_collected = obj.samples_collected + 1;

            if ~obj.estimated && mod(obj.samples_collected, 100) == 0
                fprintf("Samples collected %f / %f\n", obj.samples_collected, obj.nr_samples);
            end
            
            if obj.samples_collected == obj.nr_samples && ~obj.estimated
                data = obj.data(1:2500);

                data = data - mean(data);
                %figure; plot(data); hold on;
                
                tic
                filtfilted = filtfilt(obj.lpf, obj.A, data);
                toc
                figure; plot(filtfilted); hold on;
                
                tic
                data = filter(obj.lpf, obj.A, data);
                toc
                plot(data); hold on;

                downsampled = data(1:10:end);



                [estimated_phases, estimated_amplitudes] = phastimate(downsampled', obj.bpf, obj.EDGE, obj.AR_ORDER, obj.HILBERTWIN);

                s = size(estimated_phases);                
                fprintf("num of estimated phases: %f, %f\n", s(1), s(2));
                s = size(estimated_amplitudes);
                fprintf("num of estimated amplitudes: %f, %f\n", s(1), s(2));
                
                
                fprintf("Estimation done\n");
                obj.estimated = true;
                
                fclose(obj.lpf_file_id);

                
                [~, xline_index] = min(abs(estimated_phases-0));
                
                start_of_estimation = 250 - 32;
                end_of_estimation = 250 + 32;
                data = obj.data(1:5000);
                data = data - mean(data);
                data = filter(obj.lpf, obj.A, data);
                downsampled = data(1:10:end);
                data_filtered = filtfilt(obj.bpf, double(1), double(downsampled));
                amplitudes = zeros(500, 1);
                phases = zeros(500, 1);

                % disp(data_filtered);

                for i=1:500
                    
                    fprintf(obj.c3_file_id, "%f\n", data_filtered(i));

                    if i > start_of_estimation && i < end_of_estimation
                        estimate_index = i - start_of_estimation;
                        
                        phase = estimated_phases(estimate_index);
                        ampl = estimated_amplitudes(estimate_index);
                        
                        fprintf(obj.ampl_file_id, "%f\n", ampl);
                        fprintf(obj.phase_file_id, "%f\n", phase);
                        
                        amplitudes(i) = ampl;
                        phases(i) = phase;
                    else
                        fprintf(obj.ampl_file_id, "%f\n", 0);
                        fprintf(obj.phase_file_id, "%f\n", 0);
                        amplitudes(i) = 0;
                        phases(i) = 0;
                    end
                end
                
                figure;plot(data_filtered);hold on; plot(amplitudes); %hold on; xline(start_of_estimation + xline_index); hold on; xline(250);
                figure;plot(phases); %hold on; xline(start_of_estimation + xline_index); hold on; yline(0);  hold on; xline(250);

                fprintf("Saved ampl and phase values\n");
                fclose(obj.ampl_file_id);
                fclose(obj.phase_file_id);
                fclose(obj.c3_file_id);
            end
            obj.set_commands([]);
            
        end
        function on_end_experiment(obj)
            obj.commands = [];
        end
    end
end

