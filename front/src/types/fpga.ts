import ROSLIB from 'roslib'
import { ChannelErrorMessage, SystemErrorMessage } from './fpgaErrors'

export type Feedback =
  | PulseFeedbackMessage
  | ChargeFeedbackMessage
  | DischargeFeedbackMessage
  | SignalOutFeedbackMessage

interface Error {
  value: number
}

export interface PulseFeedbackMessage extends ROSLIB.Message {
  id: number
  error: Error
  type: 'pulse'
}

export interface ChargeFeedbackMessage extends ROSLIB.Message {
  id: number
  error: Error
  type: 'charge'
}

export interface DischargeFeedbackMessage extends ROSLIB.Message {
  id: number
  error: Error
  type: 'discharge'
}

export interface SignalOutFeedbackMessage extends ROSLIB.Message {
  id: number
  error: Error
  type: 'signalOut'
}

export interface SystemStateMessage extends ROSLIB.Message {
  channel_states: ChannelState[]

  system_error_cumulative: SystemErrorMessage
  system_error_current: SystemErrorMessage
  system_error_emergency: SystemErrorMessage

  startup_error: StartupErrorMessage

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

export interface StartupErrorMessage {
  value: number
}

export interface ChannelState {
  channel_index: number
  voltage: number
  temperature: number
  pulse_count: number
  channel_error: ChannelErrorMessage
}
