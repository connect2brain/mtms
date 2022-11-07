import ROSLIB from 'roslib'

export interface EegDatapoint {
  eeg_channels: number[]
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
