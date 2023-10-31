import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'ros/ros'

interface EegInfo extends Message {
  sampling_frequency: number
  num_of_eeg_channels: number
  num_of_emg_channels: number
  send_trigger_as_channel: boolean
}

interface EegStatistics extends Message {
  num_of_raw_samples: number
  num_of_preprocessed_samples: number
  preprocessing_time_max: number
  preprocessing_time_q95: number
  preprocessing_time_median: number
}

interface EegContextType {
  eegInfo: EegInfo | null
  eegStatistics: EegStatistics | null
}

const defaultEegState: EegContextType = {
  eegInfo: null,
  eegStatistics: null,
}

export const EegContext = React.createContext<EegContextType>(defaultEegState)

interface EegProviderProps {
  children: ReactNode
}

export const EegProvider: React.FC<EegProviderProps> = ({ children }) => {
  const [eegInfo, setEegInfo] = useState<EegInfo | null>(null)
  const [eegStatistics, setEegStatistics] = useState<EegStatistics | null>(null)

  useEffect(() => {
    /* Subscriber for EEG info. */
    const eegInfoSubscriber = new Topic<EegInfo>({
      ros: ros,
      name: '/eeg/info',
      messageType: 'eeg_interfaces/EegInfo',
    })

    eegInfoSubscriber.subscribe((message) => {
      setEegInfo(message)
    })

    /* Subscriber for EEG statistics. */
    const eegStatisticsSubscriber = new Topic<EegStatistics>({
      ros: ros,
      name: '/eeg/statistics',
      messageType: 'eeg_interfaces/EegStatistics',
    })

    eegStatisticsSubscriber.subscribe((message) => {
      setEegStatistics(message)
    })

    /* Unsubscribers */
    return () => {
      eegInfoSubscriber.unsubscribe()
      eegStatisticsSubscriber.unsubscribe()
    }
  }, [])

  return <EegContext.Provider value={{ eegInfo, eegStatistics }}>{children}</EegContext.Provider>
}
