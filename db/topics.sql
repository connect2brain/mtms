CREATE TABLE topics (
    name varchar(255),
    type varchar(255),
    activex_control_name varchar(255),
    latch int
);

--Intensity
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('intensity', 'parameter', 'New Intensity', 1);

--Inter-trial interval
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('iti', 'parameter', 'New ITI', 1);

--Number of stimuli
INSERT INTO topics (name, type, activex_control_name, latch)
  VALUES ('number_of_stimuli', 'parameter', 'New Number of stimuli', 1);

--EEG data
INSERT INTO topics (name, type, latch)
  VALUES ('eeg_data', 'stream', 0);
