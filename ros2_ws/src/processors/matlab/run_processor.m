function out = run_processor(window_size, channel_count, data_sample, time_us, first_sample_of_experiment)
    obj = MatlabProcessor(window_size, channel_count);
    %obj.enqueue(data_sample);
    data = obj.data_received(data_sample, time_us, first_sample_of_experiment);
    % display(data);
    events = obj.end_experiment();
    out = data(2);
end