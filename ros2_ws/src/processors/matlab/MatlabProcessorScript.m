function [updated_data, events] = MatlabProcessorScript(data, new_data_point)
    if length(data) < 50
        data(end + 1) = new_data_point;
    else
        for i=1:length(data) - 1
            data(i) = data(i + 1);
        end
        data(end) = new_data_point;
    end
    
    
    event = create_command("pulse_event", 0);

    updated_data = data;
    events = [event];
end