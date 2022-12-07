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

        durations_file_id

        phases
        phase_count
        max_phase_count
        saved

        sample_duration
        downsample_ratio
    end

    methods
        function obj = constructor(obj)            
            obj.A = 1;
            obj.HILBERTWIN = 64;
            obj.EDGE = 35;
            obj.AR_ORDER = 15;
            
            obj.FS = 5000;
            obj.sample_duration = 1 / obj.FS;

            obj.downsample_ratio = 10;

            obj.offset_correction = obj.FS * 0.008;
            obj.nr_seconds = 1;
            obj.nr_samples = obj.nr_seconds * obj.FS / 2; % only half of data needed to estimate future samples

            obj.lpf = firls(80, [0 80 250 5000/2]/(5000/2), [1 1 0 0], [1 1]);
            obj.bpf = firls(80, [0 6 9 13 16 (500/2)]/(500/2), [0 0 1 1 0 0], [1 1 1]);

            obj.set_channel_count(1);
            obj.set_window_size(obj.FS);
            obj.set_auto_enqueue(false);
            
            obj.estimated = false;
            obj.print_c3 = false;
            obj.samples_collected = 0;

            obj.isi_seconds = 6;
            obj.isi_samples = obj.FS * obj.isi_seconds;

            obj.target_voltage = 500;

            obj.durations_file_id = fopen("durations.csv", "w");

            fprintf(obj.durations_file_id, "estimated_phase,phase_diff\n");
            obj.max_phase_count = 100;
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
            

            if obj.samples_collected == obj.nr_samples && ~obj.estimated
                dims = size(obj.data);
                fprintf("dims: ");
                
                for i=1:numel(dims)
                    fprintf("%f,", dims(i));
                end

                fprintf("\n");
                data = obj.data(1:2500);
                data = data - mean(data);
                data = filter(obj.lpf, obj.A, data);
                fprintf("downsmaple ratio: %f\n", obj.downsample_ratio);

                downsampled = data(1:10:end);
                %downsampled = downsample(data, ratio);

                [estimated_phases, estimated_amplitudes] = phastimate(downsampled', obj.bpf, obj.EDGE, obj.AR_ORDER, obj.HILBERTWIN);

                nof_estimated_samples = numel(estimated_phases);
                future_samples = estimated_phases(nof_estimated_samples / 2 + 1:end);

                [~, index_of_peak] = min(abs(future_samples - 0));
                phase_at_peak = future_samples(index_of_peak);
                
                index_of_peak = double(obj.events_sent);
                event_time = time + (index_of_peak * 10 * obj.sample_duration - obj.offset_correction * obj.sample_duration);
                
                signal_out_event = create_signal_out_command(obj.events_sent + 1, 1, 1000, 0, event_time);
                %charge_event = create_charge_command(obj.events_sent + 2, 1, 0, event_time + 1, obj.target_voltage);
                obj.set_commands([signal_out_event]);

                fprintf("EEG time:  %f\n", time);
                fprintf("Event time %f at index %f\n", event_time, index_of_peak);
                % fprintf("Timed charge at %f\n", event_time + 1);

                obj.estimated = true;

            else
                obj.set_commands([]);
            end
            
        end
        function on_end_experiment(obj)
            charge = create_discharge_command(obj.events_sent + 1, 1, 2, 0, 0);
            obj.set_commands(charge);
            fclose(obj.durations_file_id);
        end
    end
end

