classdef ROSProcessor < handle
    
    properties
        node
        eeg_data_subscriber
        processor
        data_received
    end
    
    methods
        function obj = ROSProcessor()
            setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp");
            obj.node = ros2node("ros_processor");

            obj.eeg_data_subscriber = ros2subscriber(obj.node, "/eeg/raw_data", "mtms_interfaces/EegDatapoint", @obj.eeg_data_callback);
            obj.processor = MatlabProcessor();
            obj.data_received = 0;
        end
        
        function out = eeg_data_callback(obj, message)
            obj.data_received = obj.data_received + 1;

            channel_data = message.eeg_channels;
            time = message.time;
            first_sample_of_experiment = message.first_sample_of_experiment;

            %tic
            if obj.data_received <= 5000
                obj.processor.data_received(channel_data, time, first_sample_of_experiment);
            end
            %toc
        end
    end
end

