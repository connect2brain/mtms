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

/* State */

--Boolean

--Recharging
INSERT INTO topics (name, type, latch)
  VALUES ('charging', 'state', 1);

--Stimulating
INSERT INTO topics (name, type, latch)
  VALUES ('stimulating', 'state', 1);

--Ready to stimulate
INSERT INTO topics (name, type, latch)
  VALUES ('ready_to_stimulate', 'state', 1);

--Numeric

--Train count
INSERT INTO topics (name, type, latch)
  VALUES ('train_count', 'state', 1);

--Error code
INSERT INTO topics (name, type, latch)
  VALUES ('error_code', 'state', 1);

--Sequence counter
INSERT INTO topics (name, type, latch)
  VALUES ('sequence_counter', 'state', 1);

--Sequence length
INSERT INTO topics (name, type, latch)
  VALUES ('sequence_length', 'state', 1);

/* Data */

--EEG data
INSERT INTO topics (name, type, latch)
  VALUES ('eeg_data', 'stream', 0);
