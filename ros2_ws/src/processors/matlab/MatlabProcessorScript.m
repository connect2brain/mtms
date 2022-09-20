function [updated_data, events] = MatlabProcessorScript(data, new_data_point)
    rows = 50;
    columns = 62;

    if length(data) < rows
        data(end + 1, :) = new_data_point;
    else
        for i=1:rows - 1
            data(i, :) = data(i + 1, :);
        end
        data(end, :) = new_data_point;
    end
   
    c3_col = data(:,5);
    c3 = c3_col(end);

    [signals,avgFilter,stdFilter] = ThresholdingAlgo(c3_col,30, 5.5, 0.7);
    
    peak = signals(end);
    peak_mark = "f";
    if peak ~= 0
        peak_mark = "t";
    end
    %file_id = fopen("eeg_purematlab.csv", "a+");
    % fprintf(obj.file_id, "c3,filtered,peak\n");
    %fprintf(file_id, "%6.2f,%f,%s\n", c3, 1, peak_mark);
    %fclose(file_id);


    pulse = create_command(1, "pulse_event", 500);
    charge = create_command(2, "charge_event", 500);
    updated_data = data;
    events = [pulse, charge];

end