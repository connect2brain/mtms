function out = run_processor(window_size, channel_count, data_sample, time, first_sample_of_session)

    % HACK: to make variable size data samples
    actual_window_size = uint32(20);

    if window_size < 50
        actual_window_size = window_size;
    elseif window_size > 100
        actual_window_size = uint32(40);
    else
        actual_window_size = uint32(window_size);
    end

    actual_data_sample = zeros(actual_window_size, 1);

    if actual_window_size > 10
        for i=1:actual_window_size
            actual_data_sample(i) = i;
        end
    else
        for i=1:actual_window_size
            actual_data_sample(i) = i * 5;
        end
    end
    
    obj = MatlabProcessor();

    obj.set_auto_enqueue(true);
    
    obj.set_window_size(actual_window_size);
    obj.set_channel_count(channel_count);

    ring_buf = ring_buffer(3, 5);
    rot_buf = rotating_buffer(3,5);

    ring_buf = ring_buffer(actual_window_size, channel_count);
    rot_buf = rotating_buffer(actual_window_size, channel_count);


    obj.enqueue(actual_data_sample);

    for i=1:10
        number = randi([20 150]);
        sample = zeros(number, 1);
        for j=1:number
            sample(j) = i * j;
        end
        obj.enqueue(sample);
    end


    [commands, eeg_samples]= obj.init_session();

    [commands, eeg_samples] = obj.data_received(actual_data_sample, time, first_sample_of_session);
    actual_data_sample2 = ones(actual_window_size, 1);
    [commands, eeg_samples]= obj.data_received(actual_data_sample2, time + 1, false);
    
    data = obj.get_data();
    s = sum(data);


    data = obj.end_session();

    auto_enqueue = obj.auto_enqueue;

    ring_buf = obj.data;
    
    in_progress = obj.session_in_progress;

    % out = data(2);
    time = obj.last_sample_received_at;
    session_started = obj.session_in_progress;

    
    ring_buf.append([1.2, 0.2, 1.5, 4, 1000.9]);
    rot_buf.append([1.2, 0.2, 1.5, 4, 1000.9]);
    
    if window_size < 10
        ring_buf = ring_buf.set_buffer(zeros(2, 2));
        rot_buf = rot_buf.set_buffer(zeros(2, 2));
    else
        ring_buf = ring_buf.set_buffer(zeros(2, 5));
        rot_buf = rot_buf.set_buffer(zeros(2, 5));
    end

    for i = 1:window_size
        d = ones(actual_window_size, i);
        ring_buf = ring_buf.set_buffer(d);
        rot_buf = rot_buf.set_buffer(d);
    end
    
    for i = 1:window_size
        ring_buf.append(actual_data_sample);
        rot_buf.append(actual_data_sample);
        obj.enqueue(actual_data_sample);
    end


    out = obj.get_data();
    
end
