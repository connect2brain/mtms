
export interface Experiment {
  description: string
  pulseSequence: PulseSequence
}

export interface PulseSequence {
  nofTrains: number
  nofBurstsInTrains: number
  nofPulsesInBursts: number
  channelInfo: ChannelInfo[]
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