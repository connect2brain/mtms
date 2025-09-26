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

        function pulse_strain = calculate_strain(obj,max_currents)
            assert(size(max_currents,2)==5,"Pulse strain calculation requires five values")

            coil_5_strain = (sqrt(0.9.*max_currents(:,1).^2+max_currents(:,2).^2).*0.2...
           + sqrt(0.9.*max_currents(:,3).^2+max_currents(:,4).^2)).*max_currents(:,5);


            coil_1_strain = (0.3.*max_currents(:,2)...
                   +0.7*sqrt(max_currents(:,3).^2+0.9*max_currents(:,4).^2)...
                   +0.2.*max_currents(:,5)).*max_currents(:,1);
            
            pulse_strain = max([coil_5_strain,coil_1_strain]);
        end

        function strain_limit = calculate_strain_limit_from_load_voltages(obj,load_voltages)
            % Assume 60 us rise waveform
            waveforms{1} = struct('mode','r','duration',60*1e-6);
            waveforms = repmat(waveforms,1,5);
            
            max_currents = [];
            for i = 1:length(waveforms)
                obj.approximator.select_coil(i);
                state_trajectory = obj.approximator.generate_state_trajectory_from_waveform(abs(load_voltages(i)),waveforms{i});
                max_currents(i) = max(abs([state_trajectory.I_coil]));
            end

            strain_limit = obj.calculate_strain(max_currents);
        end

        function [strain_ok, stimulation_intensity_multiplier] = check_pulse_strain(obj,load_voltages,waveforms)
                N_pulses = size(load_voltages,1);
                max_currents = zeros(size(load_voltages));

                for i = 1:N_pulses
                    for j = 1:length(waveforms)
                        obj.approximator.select_coil(j);
                        state_trajectory = obj.approximator.generate_state_trajectory_from_waveform(abs(load_voltages(i,j)),waveforms{i,j});
                        max_currents(i,j) = max(abs([state_trajectory.I_coil]));
                    end
                end

                pulse_strain = obj.calculate_strain(max_currents);

                strain_ok = pulse_strain < obj.strain_limit;
                stimulation_intensity_multiplier = sqrt(obj.strain_limit / pulse_strain);
        end

    end
end