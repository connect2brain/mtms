function waveform = algorithm_hold_both_ends(obj, parameter, total_duration, mode_info)
    current_mode = mode_info.current_mode;
    is_last_segment = mode_info.is_last_segment;
    segment_index = mode_info.segment_index;
    mode_index = mode_info.mode_index;

    if current_mode == 'r'
        waveform = struct( ...
            'mode', {'h', 'r', 'h'}, ...
            'duration', {(total_duration - parameter) / 2, parameter, (total_duration - parameter) / 2});

    elseif current_mode == 'f'
        waveform = struct( ...
            'mode', {'h', 'f', 'h'}, ...
            'duration', {(total_duration - parameter) / 2, parameter, (total_duration - parameter) / 2});

    elseif current_mode == 'h'
        waveform = struct( ...
            'mode', {'h'}, ...
            'duration', {total_duration});
    end
end
