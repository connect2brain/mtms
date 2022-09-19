function out = run_processor(window_size, channel_count, data_sample, time_us, first_sample_of_experiment)
    actual_window_size = uint32(20);
    
    if window_size < 50
        actual_window_size = window_size;
    elseif window_size > 100
        actual_window_size = uint32(40);
    else
        actual_window_size = uint32(window_size);
    end
    actual_data_sample = zeros(actual_window_size, 1);
    
    obj = MatlabProcessor();
    
    obj.set_window_size(actual_window_size);
    obj.set_channel_count(channel_count);

    %obj.enqueue(data_sample);
    data = obj.init_experiment();

    data = obj.data_received(actual_data_sample, time_us, first_sample_of_experiment);
    % display(data);
    data = obj.end_experiment();

    auto_enqueue = obj.auto_enqueue;
    
    in_progress = obj.experiment_in_progress;

    % out = data(2);
    time_us = obj.last_sample_received_at_us;
    experiment_started = obj.experiment_in_progress;


    if window_size < 10
        obj = obj.setData(1:10);
    else
        obj = obj.setData([]);
    end
    for i = 1:window_size
        obj = obj.setData([obj.getData(), double(i)]);
    end

    out = obj.getData();
    
end