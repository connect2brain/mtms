CREATE TABLE topics (
    name varchar(255),
    type varchar(255),
    latch int
);

/* Parameters */

--Intensity
INSERT INTO topics (name, type, latch)
  VALUES ('intensity', 'parameter', 1);

--Inter-trial interval
INSERT INTO topics (name, type, latch)
  VALUES ('iti', 'parameter', 1);

--Number of stimuli
INSERT INTO topics (name, type, latch)
  VALUES ('number_of_stimuli', 'parameter', 1);

/* Commands */

--Recharge
INSERT INTO topics (name, type, latch)
  VALUES ('recharge', 'command', 0);

--Stimulate
INSERT INTO topics (name, type, latch)
  VALUES ('stimulate', 'command', 0);

--Abort
INSERT INTO topics (name, type, latch)
  VALUES ('abort', 'command', 0);

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

--Charging times
INSERT INTO topics (name, type, latch)
  VALUES ('charging_times', 'state', 1);

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

/* Planner */

--Add point
INSERT INTO topics (name, type, latch)
  VALUES ('point.add', 'command', 0);

/* Calibration */

--Set fiducial
INSERT INTO topics (name, type, latch)
  VALUES ('calibration.set_fiducial', 'command', 0)
