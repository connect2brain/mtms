import { Target } from './target'

export interface ExperimentMessage {
  description: string
  pulseSequence: PulseSequence
}

export interface PulseSequence {
  name: string
  comment: string
  nofTrains: number
  visible: boolean
  selected: boolean
  nofBurstsInTrains: number
  nofPulsesInBursts: number
  channelInfo: ChannelInfo[]
  iti: number
  ibi: number
  isis: number[]
  pulses: Pulse[]
}

export interface Pulse {
  target: Target
  intensity: number
  isi: number
  modeDuration: number
}

interface ChannelInfo {
  channelIndex: number
  voltage: number
}

export interface ChannelInfoWithEnabled {
  channelIndex: number
  voltage: number
  enabled: boolean
}
