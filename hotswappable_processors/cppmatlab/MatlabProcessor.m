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
        function on_data_received(obj, channel_data, time, first_sample_of_experiment)
            c3 = channel_data(5) - 0.25 * (channel_data(21) + channel_data(23) + channel_data(25) + channel_data(27));
            obj.enqueue(c3);
            obj.samples_collected = obj.samples_collected + 1;

            if ~obj.estimated && mod(obj.samples_collected, 100) == 0
                fprintf("Samples collected %f / %f\n", obj.samples_collected, obj.nr_samples);
            end
            
            if obj.samples_collected == obj.nr_samples && ~obj.estimated
                data = obj.data(1:2500);

                data = data - mean(data);
                
                fprintf("Low pass filtering\n");
                tic
                data = filter(obj.lpf, obj.A, data);
                toc

                downsampled = data(1:10:end);

                fprintf("Phastimating\n");
                [estimated_phases, estimated_amplitudes] = phastimate(downsampled', obj.bpf, obj.EDGE, obj.AR_ORDER, obj.HILBERTWIN);
                obj.estimated = true;
                
                nof_estimated_samples = numel(estimated_phases);
                future_samples = estimated_phases(nof_estimated_samples / 2:end);

                [~, index_of_peak] = min(abs(future_samples - 0));
                event_time = time_us + index_of_peak * (1 / obj.FS);
                pulse_event = create_command(obj.events_sent, "pulse", event_time, 500);
                obj.set_commands(pulse_event);
            else
                obj.set_commands([]);
            end
            
        end
        function on_end_experiment(obj)
            obj.commands = [];
        end
    end
end

