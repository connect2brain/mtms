classdef PeakMatlabProcessor < AbstractMatlabProcessor

    properties(Access=private)
        file_id
        peak_detection
        peak_over
    end

    methods
        function obj = constructor(obj)
            obj.set_channel_count(62);
            obj.set_window_size(50);
            obj.set_auto_enqueue(true);
            obj.peak_detection = Thresholding(zeros(obj.window_size), obj.window_size - 1, 4.5, 0.5);
        end
        function on_init_experiment(obj)
            obj.commands = [];
        end
        function on_data_received(obj, channel_data, time, first_sample_of_experiment)
            c3 = channel_data(5);

            signal = obj.peak_detection.thresholding_algo(c3);
            if signal ~=0 
                obj.peak_over = false;
            else
                obj.peak_over = true;
            end
            
            pulse = create_command(obj.events_sent + 1, "pulse_event", 0);
            charge = create_command(obj.events_sent + 2, "charge", 1200);
            obj.set_commands([pulse, charge]);
        end
        function on_end_experiment(obj)
            obj.commands = [];
        end
    end
end

