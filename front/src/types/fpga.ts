import ROSLIB from 'roslib'
import {
  ChannelErrorMessage,
  ChargeError,
  DischargeError,
  PulseError,
  SignalOutError,
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

  startup_error: StartupErrorMessage

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

export interface StartupErrorMessage {
  value: number
  NO_ERROR: number
  UART_INITIALIZATION_ERROR: number
  BOARD_STARTUP_ERROR: number
  BOARD_STATUS_MESSAGE_ERROR: number
  SAFETY_MONITOR_ERROR: number
  DISCHARGE_CONTROLLER_ERROR: number
  CHARGER_ERROR: number
  SENSORBOARD_ERROR: number
  DISCHARGE_CONTROLLER_VOLTAGE_ERROR: number
  CHARGER_VOLTAGE_ERROR: number
  IGBT_FEEDBACK_ERROR: number
  TEMPERATURE_SENSOR_PRESENCE_ERROR: number
  COIL_MEMORY_PRESENCE_ERROR: number
}

export interface ChannelState {
  channel_index: number
  voltage: number
  temperature: number
  pulse_count: number
  channel_error: ChannelErrorMessage
}
