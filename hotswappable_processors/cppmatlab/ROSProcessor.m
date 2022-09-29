classdef ROSProcessor < handle
    %ROSPROCESSOR Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        node
        eeg_data_subscriber
        processor
    end
    
    methods
        function obj = ROSProcessor()
            %ROSPROCESSOR Construct an instance of this class
            %   Detailed explanation goes here
            setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp");
            obj.node = ros2node("ros_processor");

            obj.eeg_data_subscriber = ros2subscriber(obj.node, "/eeg/raw_data", "mtms_interfaces/EegDatapoint", @obj.eeg_data_callback);
            obj.processor = MatlabProcessor();
        end
        
        function out = eeg_data_callback(obj, message)
            channel_data = message.channel_datapoint;
            time_us = message.time;
            first_sample_of_experiment = message.first_sample_of_experiment;

            %tic
            obj.processor.data_received(channel_data, time_us, first_sample_of_experiment);
            %toc
        end
        function wait_forever(obj)
            %wait_forever Wait forever
            while true
                pause(0.00001);
            end
        end
    end
end

