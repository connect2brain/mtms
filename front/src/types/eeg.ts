import ROSLIB from 'roslib'

export interface EegDatapoint {
  channel_datapoint: number[]
  time: number
  first_sample_of_experiment: boolean
}

export interface EegDatapointMessage extends ROSLIB.Message {
  channel_datapoint: number[]
  time: number
  first_sample_of_experiment: boolean
}

export interface EegTrigger {
  index: number
  time_us: number
}

export interface EegTriggerMessage extends ROSLIB.Message {
  index: number
  time_us: number
}
