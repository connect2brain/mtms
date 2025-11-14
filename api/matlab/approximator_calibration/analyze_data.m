% plot
filename = "pulse_data_coil4";

load(filename,'measurement');

n_waveforms = size(measurement,1);
n_load_voltages = size(measurement,2);
n_repetitions = size(measurement,3);



for i = 1:n_waveforms
    f = figure;
    tiledlayout("TileSpacing","tight")
    nexttile; hold on
    for j = 1:n_load_voltages
        %nexttile; hold on
        for k = 1:n_repetitions
            time_to_plot = measurement(i,j,k).data.time_current;
            %data_to_plot = measurement(i,j,k).data.current;
            data_to_plot = measurement(i,j,k).data.voltage;
            plot(time_to_plot,data_to_plot,'LineWidth',2)
            title(string(measurement(i,j,k).load_voltage))
        end
    end
end
