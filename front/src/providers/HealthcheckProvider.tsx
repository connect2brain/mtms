import React, { useState, useEffect, ReactNode } from 'react'
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
  mepHealthcheck: Healthcheck | null
}

const defaultHealthcheckState: HealthcheckContextType = {
  eegHealthcheck: null,
  mtmsDeviceHealthcheck: null,
  mepHealthcheck: null
}

export const HealthcheckContext = React.createContext<HealthcheckContextType>(defaultHealthcheckState)

interface HealthcheckProviderProps {
  children: ReactNode
}

export const HealthcheckProvider: React.FC<HealthcheckProviderProps> = ({ children }) => {
  const [eegHealthcheck, setEegHealthcheck] = useState<Healthcheck | null>(null)
  const [mtmsDeviceHealthcheck, setMtmsDeviceHealthcheck] = useState<Healthcheck | null>(null)
  const [mepHealthcheck, setMepHealthcheck] = useState<Healthcheck | null>(null)

  useEffect(() => {
    const eegSubscriber = new Topic<Healthcheck>({
      ros: ros,
      name: '/eeg/healthcheck',
      messageType: 'system_interfaces/Healthcheck'
    })

    const mtmsSubscriber = new Topic<Healthcheck>({
      ros: ros,
      name: '/mtms_device/healthcheck',
      messageType: 'system_interfaces/Healthcheck'
    })

    const mepSubscriber = new Topic<Healthcheck>({
      ros: ros,
      name: '/mep/healthcheck',
      messageType: 'system_interfaces/Healthcheck'
    })

    let eegTimeout: NodeJS.Timeout | null = null
    let mtmsTimeout: NodeJS.Timeout | null = null
    let mepTimeout: NodeJS.Timeout | null = null

    eegSubscriber.subscribe((message) => {
      setEegHealthcheck(message)
      if (eegTimeout) {
        clearTimeout(eegTimeout)
      }
      eegTimeout = setTimeout(() => {
        setEegHealthcheck(null)
      }, 1000)
    })

    mtmsSubscriber.subscribe((message) => {
      setMtmsDeviceHealthcheck(message)
      if (mtmsTimeout) {
        clearTimeout(mtmsTimeout)
      }
      mtmsTimeout = setTimeout(() => {
        setMtmsDeviceHealthcheck(null)
      }, 1000)
    })

    mepSubscriber.subscribe((message) => {
      setMepHealthcheck(message)
      if (mepTimeout) {
        clearTimeout(mepTimeout)
      }
      mepTimeout = setTimeout(() => {
        setMepHealthcheck(null)
      }, 1000)
    })

    return () => {
      eegSubscriber.unsubscribe()
      mtmsSubscriber.unsubscribe()
      mepSubscriber.unsubscribe()
      if (eegTimeout) {
        clearTimeout(eegTimeout)
      }
      if (mtmsTimeout) {
        clearTimeout(mtmsTimeout)
      }
      if (mepTimeout) {
        clearTimeout(mepTimeout)
      }
    }
  }, [])

  return (
    <HealthcheckContext.Provider value={{ eegHealthcheck, mtmsDeviceHealthcheck, mepHealthcheck }}>
      {children}
    </HealthcheckContext.Provider>
  )
}
