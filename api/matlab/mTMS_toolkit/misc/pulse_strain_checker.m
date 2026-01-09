classdef pulse_strain_checker < handle

    properties
        approximator
        strain_limit
    end
    
    methods
        function obj = pulse_strain_checker(approximator)
            obj.approximator = approximator;
            obj.strain_limit = obj.calculate_strain_limit_from_load_voltages([0,0,0,1200,1200]);
        end

        function pulse_strain = calculate_strain(obj,current_waveforms)
            assert(size(current_waveforms,2)==5,"Pulse strain calculation requires N by 5 matrix.")

            coil_5_strain = sum((sqrt(0.9.*current_waveforms(:,1).^2+current_waveforms(:,2).^2).*0.2...
           + sqrt(0.9.*current_waveforms(:,3).^2+current_waveforms(:,4).^2)).*current_waveforms(:,5));


            coil_1_strain = sum((0.9.*current_waveforms(:,2)...
                   +0.7*sqrt(current_waveforms(:,3).^2+0.9*current_waveforms(:,4).^2)...
                   +0.2.*current_waveforms(:,5)).*current_waveforms(:,1));
            
            pulse_strain = max([coil_5_strain,coil_1_strain]);
        end

        function strain_limit = calculate_strain_limit_from_load_voltages(obj,load_voltages)
            % Assume pre-defined monophasic waveform
            waveforms{1} = struct('mode',{'r','h','f'},'duration',{60e-6,30e-6,37e-6});
            waveforms = repmat(waveforms,1,5);
            
            % Initialize temporary storage and max length tracker
            temp_waveforms = cell(1, length(waveforms));
            max_len = 0;
            
            for i = 1:length(waveforms)
                obj.approximator.select_coil(i);
                state_trajectory = obj.approximator.generate_state_trajectory_from_waveform(abs(load_voltages(i)),waveforms{i});
                
                % Store the trajectory data and update maximum length
                traj_data = abs([state_trajectory.I_coil]);
                temp_waveforms{i} = traj_data;
                if length(traj_data) > max_len
                    max_len = length(traj_data);
                end
            end
            
            % Create zero-padded matrix
            current_waveforms = zeros(max_len, length(waveforms));
            for i = 1:length(waveforms)
                data = temp_waveforms{i};
                current_waveforms(1:length(data), i) = data(:);
            end

            strain_limit = obj.calculate_strain(current_waveforms);
        end

        function [strain_ok, stimulation_intensity_multiplier] = check_pulse_strain(obj,load_voltages,waveforms)
                N_pulses = size(load_voltages,1);
                num_coils = 5; % Expecting 5 coils based on calculate_strain assertion

                for i = 1:N_pulses
                    % Initialize temporary storage and max length tracker for this pulse
                    temp_waveforms = cell(1, num_coils);
                    max_len = 0;
                    
                    for j = 1:num_coils
                        obj.approximator.select_coil(j);
                        state_trajectory = obj.approximator.generate_state_trajectory_from_waveform(abs(load_voltages(i,j)),waveforms{i,j});
                        
                        % Store the trajectory data and update maximum length
                        traj_data = abs([state_trajectory.I_coil]);
                        temp_waveforms{j} = traj_data;
                        if length(traj_data) > max_len
                            max_len = length(traj_data);
                        end
                    end
                    
                    % Create zero-padded matrix for this pulse
                    current_waveforms = zeros(max_len, num_coils);
                    for j = 1:num_coils
                        data = temp_waveforms{j};
                        current_waveforms(1:length(data), j) = data(:);
                    end
                    
                    pulse_strain(i) = obj.calculate_strain(current_waveforms);
                end

                strain_ok = pulse_strain <= obj.strain_limit;
                stimulation_intensity_multiplier = sqrt(obj.strain_limit ./ pulse_strain);
        end

    end
end