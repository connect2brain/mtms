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

            obj.peak_detection = Thresholding(zeros(obj.window_size, 1), 30, 5.5, 0.7);
            obj.file_id = fopen("eeg_matlab.csv", "w");
            fprintf(obj.file_id, "c3,filtered,peak\n");
            obj.peak_over = true;
            fprintf("matlab processor init\n");
        end
        function on_init_experiment(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time_us, first_sample_of_experiment)
            obj.enqueue(channel_data);
            c3 = channel_data(5);

            signal = obj.peak_detection.thresholding_algo(c3);
            if signal == 0 && ~obj.peak_over
                obj.peak_over = true;
            end

            pulse = create_command(obj.events_sent + 1, "pulse_event", 0);
            charge = create_command(obj.events_sent + 2, "charge_event", 50);
            peak_mark = "f";

            if signal == 1 && obj.peak_over
                number_of_pulses = 1;
                peak_mark = "t";
                obj.peak_over = false;
            else
                number_of_pulses = 0;
            end

            fprintf(obj.file_id, "%6.2f,%f,%s\n", c3, 1, peak_mark);
            number_of_pulses = 2;

            obj.commands = repmat(pulse, number_of_pulses, 1);
            obj.commands(1) = charge;
            obj.commands(2) = pulse;
        end
        function on_end_experiment(obj)
            fclose(obj.file_id);

            obj.commands = [];
        end
    end
end

