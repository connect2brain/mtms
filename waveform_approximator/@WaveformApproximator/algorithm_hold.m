function waveform = algorithm_hold(obj, parameter, total_duration, mode_info)
    next_mode = mode_info.next_mode;
    is_last = mode_info.is_last;
    index = mode_info.index;

    if next_mode == 'r'
        waveform = struct( ...
            'mode', {'r', 'h'}, ...
            'duration', {parameter, total_duration - parameter});

    elseif next_mode == 'f'
        waveform = struct( ...
            'mode', {'h', 'f'}, ...
            'duration', {parameter, total_duration - parameter});

    elseif next_mode == 'h'
        waveform = struct( ...
            'mode', {'h'}, ...
            'duration', {total_duration});
    end
end
