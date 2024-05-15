function error = calculate_error(obj, param, algorithm, initial_state, target_state, total_duration, mode_info)
    % Initialize cache.
    persistent cache;
    if isempty(cache)
        cache = containers.Map('KeyType', 'char', 'ValueType', 'any');
    end

    % Create a key based on the input parameters.
    key = create_cache_key(param, total_duration, mode_info);

    % Check if the result is already cached.
    if isKey(cache, key)
        error = cache(key);
        return;
    end

    % Compute the error.
    waveform = algorithm(param, total_duration, mode_info);
    final_state = obj.apply_waveform_to_state(initial_state, waveform);
    error = abs(final_state.I_coil - target_state.I_coil);

    % Cache the computed error.
    cache(key) = error;
end

function key = create_cache_key(param, total_duration, mode_info)
    paramStr = jsonencode(param);
    modeInfoStr = jsonencode(mode_info);
    key = sprintf('%s|%f|%s', paramStr, total_duration, modeInfoStr);
end
