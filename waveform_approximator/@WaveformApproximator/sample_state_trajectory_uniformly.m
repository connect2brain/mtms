function sampling_points = sample_state_trajectory_uniformly(obj, state_trajectory, num_of_intermediate_points)

    % Generate times for sampling points, based on the durations and the number of sampling points
    sampling_points = struct('time', {}, 'index', {}, 'state', {}, 'mode_info', {});

    end_time = (length(state_trajectory) - 1) * obj.time_resolution;

    sampling_times = linspace(0, end_time, 2 + num_of_intermediate_points);

    for j = 1:length(sampling_times)
        time = sampling_times(j);
        index = obj.get_index_from_time(time);
        state = state_trajectory(index);

        if j ~= length(sampling_times)
            next_time = sampling_times(j + 1);
            next_index = obj.get_index_from_time(next_time);
            next_state = state_trajectory(next_index);

            time_difference = next_time - time;
            next_state_when_holding = obj.apply_mode_to_state(state, 'h', time_difference);

            if next_state_when_holding.I_coil > next_state.I_coil
                next_mode = 'r';
            else
                next_mode = 'f';
            end
            is_last = false;
        else
            next_mode = 'h';
            is_last = true;
        end

        mode_info = struct('next_mode', next_mode, 'is_last', is_last, 'index', j);

        sampling_point = struct( ...
            'time', time, ...
            'index', index, ...
            'state', state, ...
            'mode_info', mode_info);

        sampling_points = [sampling_points, sampling_point];
    end
end
