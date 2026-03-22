function plot_mep_data(readout, mep_configuration, N)
    % Check if the figure exists, if not, create a new one
    persistent subplot_data f subplot_time pulse_index

    if isempty(f) || ~isvalid(f)  % If figure doesn't exist, create it
        f = figure;
        set(f,'Position',[2667,796,1534,465])
        pulse_index = 0;
    end

    % Reset persistent variables when N changes
    if isempty(subplot_data) || length(subplot_data) ~= N
        subplot_data = cell(1, N);  % Initialize an array to hold subplot data
        subplot_time = cell(1, N);
        pulse_index = 0;
    end

    % Set up figure
    figure(f);  % Ensure the figure is the current one

    % Time window and sampling frequency
    window = [mep_configuration.mep_time_window_start, mep_configuration.mep_time_window_end];
    fs = 5000;
    time = (window(1) + (0:length(readout.buffer)-1) / fs) * 1000;  % Time in ms
    
    % Shift existing subplots to the left
    subplot_data = circshift(subplot_data, -1);  % Shift all elements to the left
    subplot_time = circshift(subplot_time, -1);
    
    % Insert the new data in the right-most position (end of array)
    subplot_data{end} = readout;
    subplot_time{end} = time;
    pulse_index = pulse_index + 1;

    % Create tiled layout for subplots
    t = tiledlayout(1, N);  % 1 row, N columns
    
    % Make the layout tighter by adjusting the padding and spacing
    t.TileSpacing = 'compact';
    t.Padding = 'compact';
    
    % Create subplots in the tiled layout
    for i = 1:N
        nexttile;  % Move to the i-th tile in the grid
        
        % Clear the current subplot to avoid overlapping previous plots
        cla;  % Clears the axis so no plots are drawn on top of each other
        
        if ~isempty(subplot_data{i})  % If there's data for this subplot
            % Plot the EMG data
            current_time = subplot_time{i};
            current_data_filt = subplot_data{i}.buffer_filt;
            current_data_filt = current_data_filt - mean(current_data_filt);
            current_data_raw = subplot_data{i}.buffer;
            current_data_raw = current_data_raw - mean(current_data_raw);
            plot(current_time, current_data_filt, 'LineWidth', 2);
            hold on;
            plot(current_time,current_data_raw,'LineStyle','--','Color',[0.7,0.7,0.7])

            % Mark the maximum and minimum points
            max_mep = current_data_filt(subplot_data{i}.max_ind);
            min_mep = current_data_filt(subplot_data{i}.min_ind);
            
            plot(current_time(subplot_data{i}.min_ind), min_mep, '.r', 'MarkerSize', 40);
            plot(current_time(subplot_data{i}.max_ind), max_mep, '.r', 'MarkerSize', 40);
            mep_amp = abs(max_mep-min_mep);
            title(sprintf("N = %i ; A = %.0f µV",pulse_index+i-N,mep_amp));

            % Label the axes
            if i == 1
                ylabel('EMG (µV)');
            end
            xlabel('Time (ms)');
        end
    end
end
