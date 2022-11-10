import ROSLIB from 'roslib'
import {
  ChannelErrorMessage,
  ChargeError,
  DischargeError,
  PulseError,
  SignalOutError, StartupError,
  SystemErrorMessage
} from './fpgaErrors'

export type Feedback =
  | PulseFeedbackMessage
  | ChargeFeedbackMessage
  | DischargeFeedbackMessage
  | SignalOutFeedbackMessage


export interface PulseFeedbackMessage extends ROSLIB.Message {
  id: number
  error: PulseError
  type: 'pulse'
}

export interface ChargeFeedbackMessage extends ROSLIB.Message {
  id: number
  error: ChargeError
  type: 'charge'
}

export interface DischargeFeedbackMessage extends ROSLIB.Message {
  id: number
  error: DischargeError
  type: 'discharge'
}

export interface SignalOutFeedbackMessage extends ROSLIB.Message {
  id: number
  error: SignalOutError
  type: 'signalOut'
}

export interface SystemStateMessage extends ROSLIB.Message {
  channel_states: ChannelState[]

  system_error_cumulative: SystemErrorMessage
  system_error_current: SystemErrorMessage
  system_error_emergency: SystemErrorMessage

  startup_error: StartupError

  device_state: DeviceStateMessage
  experiment_state: ExperimentStateMessage

  time: number
}

export interface ExperimentStateMessage {
  value: number
  STOPPED: number
  STARTING: number
  STARTED: number
  STOPPING: number
}

export interface DeviceStateMessage {
  value: number
  NOT_OPERATIONAL: number
  STARTUP: number
  OPERATIONAL: number
  SHUTDOWN: number
}


export interface ChannelState {
  channel_index: number
  voltage: number
  temperature: number
  pulse_count: number
  channel_error: ChannelErrorMessage
}
