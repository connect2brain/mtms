import ROSLIB from 'roslib'
import {
  ChannelErrorMessage,
  StartupError,
  SystemErrorMessage,
} from './mtmsDeviceErrors'

export const DeviceState = {
  NOT_OPERATIONAL: 0,
  STARTUP: 1,
  OPERATIONAL: 2,
  SHUTDOWN: 3,
}

export const ExperimentState = {
  STOPPED: 0,
  STARTING: 1,
  STARTED: 2,
  STOPPING: 3,
}

interface Error {
  value: number
}

export interface SystemStateMessage extends ROSLIB.Message {
  channel_states: ChannelState[]

  system_error_cumulative: SystemErrorMessage
  system_error_current: SystemErrorMessage
  system_error_emergency: SystemErrorMessage

  startup_error: Error

  device_state: DeviceStateMessage
  experiment_state: ExperimentStateMessage

  time: number
}

export interface ExperimentStateMessage {
  value: number
}

export interface DeviceStateMessage {
  value: number
}

export interface ChannelState {
  channel_index: number
  voltage: number
  temperature: number
  pulse_count: number
  channel_error: ChannelErrorMessage
}
