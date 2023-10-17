import React, { useState, useEffect, useContext, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'services/ros'

export const HealthcheckStatus = {
  READY: 0,
  NOT_READY: 1,
  ERROR: 2,
}

interface HealthcheckStatusMessage {
  value: number
}

interface Healthcheck extends ROSLIB.Message {
  status: HealthcheckStatusMessage,
  status_message: string,
  actionable_message: string
}

interface HealthcheckContextType {
  eegHealthcheck: Healthcheck | null
  mtmsDeviceHealthcheck: Healthcheck | null
}

const defaultHealthcheckState: HealthcheckContextType = {
  eegHealthcheck: null,
  mtmsDeviceHealthcheck: null
}

export const HealthcheckContext = React.createContext<HealthcheckContextType>(defaultHealthcheckState)

interface HealthcheckProviderProps {
  children: ReactNode
}

export const HealthcheckProvider: React.FC<HealthcheckProviderProps> = ({ children }) => {
  const [eegHealthcheck, setEegHealthcheck] = useState<Healthcheck | null>(null)
  const [mtmsDeviceHealthcheck, setMtmsDeviceHealthcheck] = useState<Healthcheck | null>(null)

  useEffect(() => {
    const eegSubscriber = new Topic({
      ros: ros,
      name: '/eeg/healthcheck',
      messageType: 'system_interfaces/Healthcheck'
    }) as Topic<Healthcheck>

    const mtmsSubscriber = new Topic({
      ros: ros,
      name: '/mtms_device/healthcheck',
      messageType: 'system_interfaces/Healthcheck'
    }) as Topic<Healthcheck>

    eegSubscriber.subscribe((message) => {
      setEegHealthcheck(message)
    })

    mtmsSubscriber.subscribe((message) => {
      setMtmsDeviceHealthcheck(message)
    })

    return () => {
      eegSubscriber.unsubscribe()
      mtmsSubscriber.unsubscribe()
    }
  }, [])

  return (
    <HealthcheckContext.Provider value={{ eegHealthcheck, mtmsDeviceHealthcheck }}>
      {children}
    </HealthcheckContext.Provider>
  )
}
