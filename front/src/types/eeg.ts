import ROSLIB from 'roslib'

export interface EegInfo extends ROSLIB.Message {
  sampling_frequency: number
  n_channels: number
  send_trigger_as_channel: boolean
}

export const EventTypes = {
  CHARGE: 0,
  PULSE: 1,
  DISCHARGE: 2,
  TRIGGER_OUT: 3,
}

export interface EegSample {
  eeg_data: number[]
  emg_data: number[]
  time: number
  first_sample_of_session: boolean
}

export interface EegSampleMessage extends ROSLIB.Message {
  eeg_data: number[]
  time: number
  first_sample_of_session: boolean
}

export interface EegBatch extends ROSLIB.Message {
  batch: EegSample[]
}

export interface EegBatchMessage extends ROSLIB.Message {
  batch: EegSample[]
}

export interface EegTrigger {
  index: number
  time: number
}

export interface EegTriggerMessage extends ROSLIB.Message {
  index: number
  time: number
}

export interface MTMSEvent {
  whenToExecute: number
  processingStartTime: number
  eventType: number
}

export interface MTMSEventMessage extends ROSLIB.Message {
  when_to_execute: number
  processing_start_time: number
  event_type: number
}
