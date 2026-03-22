import React, { ReactNode, useEffect, useState } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'
import { EegDeviceInfoMessage } from 'types/eeg'

interface EegDeviceInfoContextType {
  eegDeviceInfo: EegDeviceInfoMessage | null
}

const defaultEegDeviceInfoState: EegDeviceInfoContextType = {
  eegDeviceInfo: null,
}

export const EegDeviceInfoContext = React.createContext<EegDeviceInfoContextType>(defaultEegDeviceInfoState)

interface EegDeviceInfoProviderProps {
  children: ReactNode
}

export const EegDeviceInfoProvider: React.FC<EegDeviceInfoProviderProps> = ({ children }) => {
  const [eegDeviceInfo, setEegDeviceInfo] = useState<EegDeviceInfoMessage | null>(null)

  useEffect(() => {
    const eegDeviceInfoSubscriber = new ROSLIB.Topic<EegDeviceInfoMessage>({
      ros: ros,
      name: '/mtms/eeg_device/info',
      messageType: 'mtms_eeg_interfaces/msg/EegDeviceInfo',
    })

    const callback = (message: EegDeviceInfoMessage) => {
      setEegDeviceInfo(message)
    }

    eegDeviceInfoSubscriber.subscribe(callback)

    return () => {
      eegDeviceInfoSubscriber.unsubscribe(callback)
    }
  }, [])

  return <EegDeviceInfoContext.Provider value={{ eegDeviceInfo }}>{children}</EegDeviceInfoContext.Provider>
}
