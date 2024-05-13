function waveform = algorithm_falling_rising(obj, parameter, total_duration, mode_info)
    next_mode = mode_info.next_mode;
    is_last = mode_info.is_last;
    index = mode_info.index;

    if next_mode == 'r'
        waveform = struct( ...
            'mode', {'r', 'f', 'h'}, ...
            'duration', {parameter, 4e-6, total_duration - parameter - 4e-6});

    elseif next_mode == 'f'
        waveform = struct( ...
            'mode', {'h', 'r', 'f'}, ...
            'duration', {parameter, 4e-6, total_duration - parameter - 4e-6});

    elseif next_mode == 'h'
        waveform = struct( ...
            'mode', {'h'}, ...
            'duration', {total_duration});
    end
end
