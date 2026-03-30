import React, { useState, useEffect, ReactNode } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'

export const HealthcheckStatus = {
  READY: 0,
  NOT_READY: 1,
  DISABLED: 2,
  ERROR: 3,
}

interface Healthcheck extends ROSLIB.Message {
  status: number
  status_message: string
  actionable_message: string
}

interface HealthcheckContextType {
  eegHealthcheck: Healthcheck | null
  mtmsDeviceHealthcheck: Healthcheck | null
  mepHealthcheck: Healthcheck | null
  remoteControllerHealthcheck: Healthcheck | null
}

const defaultHealthcheckState: HealthcheckContextType = {
  eegHealthcheck: null,
  mtmsDeviceHealthcheck: null,
  mepHealthcheck: null,
  remoteControllerHealthcheck: null,
}

export const HealthcheckContext = React.createContext<HealthcheckContextType>(defaultHealthcheckState)

interface HealthcheckProviderProps {
  children: ReactNode
}

export const HealthcheckProvider: React.FC<HealthcheckProviderProps> = ({ children }) => {
  const [eegHealthcheck, setEegHealthcheck] = useState<Healthcheck | null>(null)
  const [mtmsDeviceHealthcheck, setMtmsDeviceHealthcheck] = useState<Healthcheck | null>(null)
  const [mepHealthcheck, setMepHealthcheck] = useState<Healthcheck | null>(null)
  const [remoteControllerHealthcheck, setRemoteControllerHealthcheck] = useState<Healthcheck | null>(null)

  useEffect(() => {
    const eegSubscriber = new ROSLIB.Topic<Healthcheck>({
      ros: ros,
      name: '/mtms/eeg/healthcheck',
      messageType: 'mtms_system_interfaces/Healthcheck',
    })

    const mtmsSubscriber = new ROSLIB.Topic<Healthcheck>({
      ros: ros,
      name: '/mtms/device/healthcheck',
      messageType: 'mtms_system_interfaces/Healthcheck',
    })

    const mepSubscriber = new ROSLIB.Topic<Healthcheck>({
      ros: ros,
      name: '/mtms/mep/healthcheck',
      messageType: 'mtms_system_interfaces/Healthcheck',
    })

    const remoteControllerSubscriber = new ROSLIB.Topic<Healthcheck>({
      ros: ros,
      name: '/mtms/remote_controller/healthcheck',
      messageType: 'mtms_system_interfaces/Healthcheck',
    })

    let eegTimeout: NodeJS.Timeout | null = null
    let mtmsTimeout: NodeJS.Timeout | null = null
    let mepTimeout: NodeJS.Timeout | null = null
    let remoteControllerTimeout: NodeJS.Timeout | null = null

    eegSubscriber.subscribe((message) => {
      setEegHealthcheck(message)
      if (eegTimeout) {
        clearTimeout(eegTimeout)
      }
      eegTimeout = setTimeout(() => {
        setEegHealthcheck(null)
      }, 1200)
    })

    mtmsSubscriber.subscribe((message) => {
      setMtmsDeviceHealthcheck(message)
      if (mtmsTimeout) {
        clearTimeout(mtmsTimeout)
      }
      mtmsTimeout = setTimeout(() => {
        setMtmsDeviceHealthcheck(null)
      }, 1200)
    })

    mepSubscriber.subscribe((message) => {
      setMepHealthcheck(message)
      if (mepTimeout) {
        clearTimeout(mepTimeout)
      }
      mepTimeout = setTimeout(() => {
        setMepHealthcheck(null)
      }, 1200)
    })

    remoteControllerSubscriber.subscribe((message) => {
      setRemoteControllerHealthcheck(message)
      if (remoteControllerTimeout) {
        clearTimeout(remoteControllerTimeout)
      }
      remoteControllerTimeout = setTimeout(() => {
        setRemoteControllerHealthcheck(null)
      }, 1200)
    })

    return () => {
      eegSubscriber.unsubscribe()
      mtmsSubscriber.unsubscribe()
      mepSubscriber.unsubscribe()
      remoteControllerSubscriber.unsubscribe()
      if (eegTimeout) {
        clearTimeout(eegTimeout)
      }
      if (mtmsTimeout) {
        clearTimeout(mtmsTimeout)
      }
      if (mepTimeout) {
        clearTimeout(mepTimeout)
      }
      if (remoteControllerTimeout) {
        clearTimeout(remoteControllerTimeout)
      }
    }
  }, [])

  return (
    <HealthcheckContext.Provider
      value={{
        eegHealthcheck,
        mtmsDeviceHealthcheck,
        mepHealthcheck,
        remoteControllerHealthcheck,
      }}
    >
      {children}
    </HealthcheckContext.Provider>
  )
}
