import { Target } from './target'

export interface ExperimentMessage {
  description: string
  pulseSequence: PulseSequence
}

export interface PulseSequence {
  nofTrains: number
  nofBurstsInTrains: number
  nofPulsesInBursts: number
  channelInfo: ChannelInfo[]
  iti: number
  ibi: number
  isis: number[]
  targets?: Target[]
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
