function waveform = algorithm_alternating_hold(obj, parameter, total_duration, mode_info)
    next_mode = mode_info.next_mode;
    is_last = mode_info.is_last;
    index = mode_info.index;

    is_even = mod(index, 2) == 0;

    if next_mode == 'r'
        if is_even
            waveform = struct( ...
                'mode', {'r', 'h'}, ...
                'duration', {parameter, total_duration - parameter});
        else
            waveform = struct( ...
                'mode', {'h', 'r'}, ...
                'duration', {parameter, total_duration - parameter});
        end

    elseif next_mode == 'f'
        if is_even
            waveform = struct( ...
                'mode', {'f', 'h'}, ...
                'duration', {parameter, total_duration - parameter});
        else
            waveform = struct( ...
                'mode', {'h', 'f'}, ...
                'duration', {parameter, total_duration - parameter});
        end

    elseif next_mode == 'h'
        waveform = struct( ...
            'mode', {'h'}, ...
            'duration', {total_duration});
    end
end
