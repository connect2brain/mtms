import ROSLIB from 'roslib'

export interface SystemStateMessage extends ROSLIB.Message {
  cumulativeError: number
  currentError: number
  emergencyError: number
  channelStates: ChannelState[]
  startupSequenceError: number
  deviceState: number
  experimentState: number
  time: number
}

export interface ChannelState {
  channelIndex: number
  voltage: number
  temperature: number
  pulseCount: number
  error: number
}
