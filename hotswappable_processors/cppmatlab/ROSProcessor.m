classdef ROSProcessor < handle
    %ROSPROCESSOR Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        node
        eeg_data_subscriber
        processor
        data_received
    end
    
    methods
        function obj = ROSProcessor()
            %ROSPROCESSOR Construct an instance of this class
            %   Detailed explanation goes here
            setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp");
            obj.node = ros2node("ros_processor");

            obj.eeg_data_subscriber = ros2subscriber(obj.node, "/eeg/raw_data", "mtms_interfaces/EegDatapoint", @obj.eeg_data_callback);
            obj.processor = MatlabProcessor();
            obj.data_received = 0;
        end
        
        function out = eeg_data_callback(obj, message)
            obj.data_received = obj.data_received + 1;

            channel_data = message.channel_datapoint;
            time_us = message.time;
            first_sample_of_experiment = message.first_sample_of_experiment;

            %tic
            if obj.data_received <= 5000
                obj.processor.data_received(channel_data, time_us, first_sample_of_experiment); 
            end
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

