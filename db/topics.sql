CREATE TABLE topics (
    name varchar(255),
    type varchar(255),
    activex_control_name varchar(255),
    latch int
);

/* Parameters */

--Intensity
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('intensity', 'parameter', 'New Intensity', 1);

--Inter-trial interval
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('iti', 'parameter', 'New ITI', 1);

--Number of stimuli
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('number_of_stimuli', 'parameter', 'New Number of stimuli', 1);

/* Commands */

--Recharge
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('recharge', 'command', 'Recharge', 0);

--Stimulate
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('stimulate', 'command', 'Stimulate', 0);

--Abort
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('abort', 'command', 'Abort stimulation', 0);

/* Data */

--EEG data
INSERT INTO topics (name, type, latch)
  VALUES ('eeg_data', 'stream', 0);
