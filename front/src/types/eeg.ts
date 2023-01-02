import ROSLIB from 'roslib'

export const EventTypes = {
  CHARGE: 0,
  PULSE: 1,
  DISCHARGE: 2,
  SIGNAL_OUT: 3,
}

export interface EegDatapoint {
  eeg_channels: number[]
  emg_channels: number[]
  time: number
  first_sample_of_experiment: boolean
}

export interface EegDatapointMessage extends ROSLIB.Message {
  eeg_channels: number[]
  time: number
  first_sample_of_experiment: boolean
}

export interface EegBatch extends ROSLIB.Message {
  batch: EegDatapoint[]
}

export interface EegBatchMessage extends ROSLIB.Message {
  batch: EegDatapoint[]
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
