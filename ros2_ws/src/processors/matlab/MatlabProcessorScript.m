function [updated_data, events] = MatlabProcessorScript(data, new_data_point)
    fprintf("data length %d\n", length(data));

    rows = 50;
    columns = 62;

    % disp(data);
    disp(size(data));
    disp(size(new_data_point));
    disp(size(data(end,:)));

    if length(data) < rows
        data(end + 1, :) = new_data_point;
    else
        for i=1:rows - 1
            data(i, :) = data(i + 1, :);
        end
        data(end, :) = new_data_point;
    end
    
    
    event = create_command("pulse_event", 0);
    disp(event);
    updated_data = data;
    events = (event);
    disp(events);
    disp(data);
end