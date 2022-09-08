function out = run_processor(window_size, data_sample, time_us, first_sample_of_experiment)
    obj = Processor(window_size);
    obj.enqueue(data_sample);
    obj.data_received(data_sample, time_us, first_sample_of_experiment);
    out = obj.data;
end