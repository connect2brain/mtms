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
  channels: ChannelInfoWithEnabled[]
  setChannels: (channels: ChannelInfoWithEnabled[]) => void
}

const useStore = create<Store>(
  devtools((set) => ({
    channels: initialChannels,
    setChannels: (channels) => set((state) => ({ channels })),
  })),
)

export default useStore
