import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'ros/ros'

interface EegInfo extends Message {
  sampling_frequency: number
  num_of_eeg_channels: number
  num_of_emg_channels: number
  send_trigger_as_channel: boolean
}

interface EegContextType {
  eegInfo: EegInfo | null
}

const defaultEegState: EegContextType = {
  eegInfo: null,
}

export const EegContext = React.createContext<EegContextType>(defaultEegState)

interface EegProviderProps {
  children: ReactNode
}

export const EegProvider: React.FC<EegProviderProps> = ({ children }) => {
  const [eegInfo, setEegInfo] = useState<EegInfo | null>(null)

  useEffect(() => {
    const eegInfoSubscriber = new Topic<EegInfo>({
      ros: ros,
      name: '/eeg/info',
      messageType: 'eeg_interfaces/EegInfo',
    })

    eegInfoSubscriber.subscribe((message) => {
      setEegInfo(message)
      console.log(message)
    })

    return () => {
      eegInfoSubscriber.unsubscribe()
    }
  }, [])

  return <EegContext.Provider value={{ eegInfo }}>{children}</EegContext.Provider>
}
