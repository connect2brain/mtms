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

        estimated
        samples_collected
        print_c3

        isi_samples
        isi_seconds

        target_voltage

        file_id
        pxx_file_id
        f_file_id

        phases
        phase_count
        max_phase_count
        saved
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

            obj.isi_seconds = 1;
            obj.isi_samples = obj.FS * obj.isi_seconds;

            obj.target_voltage = 500;

            obj.file_id = fopen("peaks.csv", "w");
            obj.pxx_file_id = fopen("pxx.csv", "w");
            obj.f_file_id = fopen("f.csv", "w");

            fprintf(obj.file_id, "estimated_phase,phase_diff\n");
            obj.max_phase_count = 1000;
            obj.phase_count = 0;
            obj.phases = zeros(obj.max_phase_count, 1);
            obj.saved = false;
            
        end
        function on_init_experiment(obj)
            charge = create_charge_command(obj.events_sent + 1, 1, 2, 0, obj.target_voltage);
            obj.set_commands(charge);
        end
        function on_data_received(obj, channel_data, time, first_sample_of_experiment)         
            if obj.estimated && obj.samples_collected == obj.isi_samples
                obj.estimated = false;
                obj.samples_collected = 0;
            end

            if ~obj.estimated
                c3 = channel_data(5) - 0.25 * (channel_data(21) + channel_data(23) + channel_data(25) + channel_data(27));
                obj.enqueue(c3);
            end

            obj.samples_collected = obj.samples_collected + 1;

            if ~obj.estimated && mod(obj.samples_collected, 100) == 0
                %fprintf("Samples collected %f / %f\n", obj.samples_collected, obj.nr_samples);
            end
            

            if obj.samples_collected == obj.nr_samples && ~obj.estimated
                data = obj.data(1:2500);

                data = data - mean(data);
                
                %fprintf("Low pass filtering\n");
                data = filter(obj.lpf, obj.A, data);

                downsampled = data(1:10:end);

                %fprintf("Phastimating\n");
                [estimated_phases, estimated_amplitudes] = phastimate(downsampled', obj.bpf, obj.EDGE, obj.AR_ORDER, obj.HILBERTWIN);

                nof_estimated_samples = numel(estimated_phases);
                future_samples = estimated_phases(nof_estimated_samples / 2 + 1:end);

                [~, index_of_peak] = min(abs(future_samples - 0));
                phase_at_peak = future_samples(index_of_peak);

                
                
                real_data = obj.data() - mean(obj.data);
                real_data = filter(obj.lpf, obj.A, real_data);
                real_data = real_data(1:10:end);

                real_phases = angle(hilbert(real_data));
                real_phase_at_estimated_peak = real_phases(2500 + index_of_peak);
                phase_diff = phase_at_peak - real_phase_at_estimated_peak;

                %fprintf("Estimated and real phase at peak: %f, %f\n", phase_at_peak, real_phase_at_estimated_peak);

                fprintf(obj.file_id, "%f, %f\n", phase_at_peak, phase_diff);

                if obj.phase_count < obj.max_phase_count
                    obj.phase_count = obj.phase_count + 1;
                    obj.phases(obj.phase_count) = phase_diff;
                    fprintf("Phases estimated: %f / %f\n", obj.phase_count, obj.max_phase_count);
                end
                if obj.phase_count >= obj.max_phase_count && ~obj.saved
                    %save("phases.mat", obj.phases);
                    %writematrix(obj.phases, "phases.csv");
                    %for i=1:obj.max_phase_count
                    %    fprintf(obj.file_id, "%f\n", obj.phases(i));
                    %end
                    %obj.saved = true;
                end


                

                event_time = time_us + index_of_peak * (1 / obj.FS) - obj.offset_correction * (1 / obj.FS);

                pulse_event = create_pulse_command(obj.events_sent + 1, 1, 0, event_time);
                charge_event = create_charge_command(obj.events_sent + 2, 1, 0, event_time + 1000000000, obj.target_voltage);
                obj.set_commands([pulse_event, charge_event]);

                %fprintf("Timed pulse at %lu\n", event_time);
                %fprintf("Timed charge at %lu\n", event_time + 1000000000);

                obj.estimated = true;

            else
                obj.set_commands([]);
            end
            
        end
        function on_end_experiment(obj)
            charge = create_discharge_command(obj.events_sent + 1, 1, 2, 0, 0);
            obj.set_commands(charge);
            fclose(obj.file_id);
        end
    end
end

