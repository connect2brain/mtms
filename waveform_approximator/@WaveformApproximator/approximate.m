function waveform = approximate(obj, actual_voltage, target_voltage, waveform, steps)
    gr = (sqrt(5) + 1) / 2;             % golden ratio
    tau = 1;                            % error tolerance

    % Get the crossing points.
    crossing_points = obj.get_crossing_points(waveform, steps);

    % Calculate the reference timecourse.
    reference_timecourse = obj.calculate_timecourse(target_voltage, waveform);

    % Generate initial conditions.
    ic = obj.generate_initial_conditions(actual_voltage, crossing_points.index(1));

    % Initialize control sequence

    % TODO: preallocate
    waveform.durations = [];
    waveform.modes = 'h';

    start_index = 0;

    for i = 1:length(crossing_points.index)
        if i == 1
            a = 1;
        else
            a = crossing_points.index(i-1);
        end
        b = crossing_points.index(i);

        c = floor(b - (b - a) / gr);
        d = floor(a + (b - a) / gr);

        while (abs(c - d) > tau)
            ic_c = obj.get_initial_conditions_by_index(ic, c-start_index);
            dur_c = floor(crossing_points.index(i)-c)*obj.resolution;
            i_c = obj.calculate_coil_current(ic_c, crossing_points.step_type(i), dur_c);

            ic_d = obj.get_initial_conditions_by_index(ic, d-start_index);
            dur_d = floor(crossing_points.index(i)-d)*obj.resolution;
            i_d = obj.calculate_coil_current(ic_d, crossing_points.step_type(i), dur_d);

            % distance from the midpoint
            delta_ic = abs(reference_timecourse.I_coil(crossing_points.index(i)) - i_c);
            delta_id = abs(reference_timecourse.I_coil(crossing_points.index(i)) - i_d);

            % Update the section of search
            if (delta_ic < delta_id)
                b = d;
            else
                a = c;
            end

            % Update iteration points
            c = floor(b - (b - a) / gr);
            d = floor(a + (b - a) / gr);
        end

        old_start_index = start_index;
        start_index = floor((a+b)/2);

        conds = obj.get_initial_conditions_by_index(ic, start_index-old_start_index);

        if i == length(crossing_points.index)
            forward_duration = 10e-6;
        elseif i == 1
            forward_duration = (crossing_points.index(i+1)-1)*obj.resolution;
        else
            forward_duration = (crossing_points.index(i+1)-crossing_points.index(i-1))*obj.resolution;
        end
        ic = obj.calculate_step(conds, crossing_points.step_type(i), forward_duration);

        waveform.modes = [waveform.modes crossing_points.step_type(i)];
        waveform.durations = [waveform.durations (start_index-old_start_index)*obj.resolution];
    end
    waveform.durations = [waveform.durations 10e-6];

    % Assert that durations are not too short; the minimum duration is 4 us.
    %
    % TODO: This should be mTMS device specific; for instance, it differs between Tubingen and Aalto.
    assert (all(waveform.durations >= 4e-6), 'The duration of the waveform is too short.');
end
