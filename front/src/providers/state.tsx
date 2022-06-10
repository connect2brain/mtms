import create from 'zustand'
import { devtools } from 'zustand/middleware'
import { ChannelInfoWithEnabled, PulseSequence } from '../types/pulseSequence'
import { Target } from '../types/target'
import targets from '../views/Targets'

const initialChannels: ChannelInfoWithEnabled[] = [
  {
    channelIndex: 1,
    enabled: true,
    voltage: 0,
  },
  {
    channelIndex: 2,
    enabled: false,
    voltage: 0,
  },
  {
    channelIndex: 3,
    enabled: false,
    voltage: 0,
  },
  {
    channelIndex: 4,
    enabled: false,
    voltage: 0,
  },
  {
    channelIndex: 5,
    enabled: false,
    voltage: 0,
  },
  {
    channelIndex: 6,
    enabled: false,
    voltage: 0,
  },
]

type Store = {
  description: string
  setDescription: (description: string) => void

  channels: ChannelInfoWithEnabled[]
  setChannels: (channels: ChannelInfoWithEnabled[]) => void

  isis: number[]
  setIsis: (isis: number[]) => void

  nofPulsesInBursts: number
  setNofPulsesInBursts: (nofPulsesInBursts: number) => void

  nofBurstsInTrains: number
  setNofBurstsInTrains: (nofBurstsInTrains: number) => void

  nofTrains: number
  setNofTrains: (nofTrains: number) => void

  iti: number
  setIti: (iti: number) => void

  ibi: number
  setIbi: (ibi: number) => void

  targets: Target[]
  setTargets: (targets: Target[]) => void

  sequences: PulseSequence[]
  setSequences: (sequences: PulseSequence[]) => void

  expandedSequences: any
  setExpandedSequences: (expandedSequences: number[]) => void
}

const useStore = create<Store>(
  devtools((set) => ({
    channels: initialChannels,
    setChannels: (channels) => set({ channels }),

    description: '',
    setDescription: (description) => set({ description }),

    isis: [3, 3],
    setIsis: (isis) => set({ isis }),

    nofPulsesInBursts: 3,
    setNofPulsesInBursts: (nofPulsesInBursts) => set({ nofPulsesInBursts }),

    nofBurstsInTrains: 3,
    setNofBurstsInTrains: (nofBurstsInTrains) => set({ nofBurstsInTrains }),

    nofTrains: 1,
    setNofTrains: (nofTrains) => set({ nofTrains }),

    iti: 500,
    setIti: (iti) => set({ iti }),

    ibi: 500,
    setIbi: (ibi) => set({ ibi }),

    targets: [],
    setTargets: (targets) => set({ targets }),

    sequences: [],
    setSequences: (sequences) => set({ sequences }),

    expandedSequences: {},
    setExpandedSequences: (expandedSequences: number[]) => set({ expandedSequences }),
  })),
)

export default useStore
