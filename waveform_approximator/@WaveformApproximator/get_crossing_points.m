% examples:
%   waveform.modes = 'rhf'                 rise,   hold,   fall    monotrapezoid
%   waveform.durations = [60, 30, 40]      60      30      40      us
%
%   steps = [4, 1, 2]                      4       1       2       steps
%
%   crossings = obj.get_crossing_points(waveform, steps)

%   waveform.modes = 'rhfhr'                       rise,   hold,   fall,   hold,   rise    biphasic
%   waveform.durations = [60, 30, 120, 30, 30]     60      30      120     30      30      us
%
%   steps = [4, 1, 2, 1, 2]                        4       1       2       1       2       steps
%
%   crossings = obj.get_crossing_points(waveform, steps)

function crossing_points = get_crossing_points(obj, waveform, steps)
    % check if the number of steps is correct
    assert (length(waveform.modes) == length(steps), 'Number of steps does not match the number of phases in the waveform');

    durations = waveform.durations;
    modes = waveform.modes;

    % find number of crossing points based on the steplist
    n_crossing_points = 0;
    for i = 1:length(steps)
        if modes(i) == 'r' || modes(i) == 'f'
            n_crossing_points = n_crossing_points + 2*steps(i);
        end
    end
    crossing_points.index = zeros(1, n_crossing_points);
    crossing_points.step_type = zeros(1, n_crossing_points);

    % determine the distances between crossing points, mark holds with a 0
    dist_between_points_list = [];
    for i = 1:length(steps)
        if modes(i) == 'h' || modes(i) == 'a'
            dist_between_points_list = [dist_between_points_list, 0];
        else
            dist_between_points_list = [dist_between_points_list, floor(durations(i)/obj.resolution/(steps(i)*2))];
        end
    end

    % go through the step list
    current_index = 0;
    for i = 1:length(steps)

        cumulated_index = 0;
        if i ~= 1
            cumulated_index = floor(sum(durations(1:i-1))/obj.resolution);
        end

        % skip all hold phases
        if modes(i) == 'h' || modes(i) == 'a'
            continue
        end

        % step consists of 2 parts: phase and hold
        for j = 1:steps(i)*2
            current_index = current_index + 1;

            % if this is the last step, place the following hold to the end
            % of this phase
            if j == steps(i)*2
                % check if next phase exists and is a hold -> place the
                % hold to the middle of the next phase
                if i ~= length(steps) && (modes(i+1) == 'h' || modes(i+1) == 'a')
                    crossing_points.index(current_index) = floor((sum(durations(1:i)) + durations(i+1)/2)/obj.resolution);
                else
                    crossing_points.index(current_index) = floor(sum(durations(1:i)/obj.resolution));
                end
                crossing_points.step_type(current_index) = 'h';

            % otherwise add crossing points as normal
            else
                crossing_points.index(current_index) = cumulated_index + j*dist_between_points_list(i);
                if rem(current_index, 2) ~= 0
                    crossing_points.step_type(current_index) = modes(i);
                else
                    crossing_points.step_type(current_index) = 'h';
                end
            end
        end
    end
end