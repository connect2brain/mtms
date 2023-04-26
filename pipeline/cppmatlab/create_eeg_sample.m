function sample = create_eeg_sample(channel_data, time, first_sample_of_experiment)
    sample.channel_data = double(channel_data);
    sample.time = double(time);
    sample.first_sample_of_experiment = logical(first_sample_of_experiment);
    
    coder.cstructname(sample, 'matlab_eeg_sample');
end
