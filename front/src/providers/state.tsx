import create from 'zustand'
import { devtools } from 'zustand/middleware'
import { ChannelInfoWithEnabled } from '../types/pulseSequence'

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
}

const useStore = create<Store>(
  devtools((set) => ({
    channels: initialChannels,
    setChannels: (channels) => set((state) => ({ channels })),

    description: '',
    setDescription: (description) => set({ description }),

    isis: [3, 3, 3],
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
  })),
)

export default useStore
