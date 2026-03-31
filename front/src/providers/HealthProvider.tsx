import React, { useState, useEffect, ReactNode } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'

export const HealthStatus = {
  READY: 0,
  DEGRADED: 1,
  ERROR: 2,
}

interface ComponentHealth extends ROSLIB.Message {
  health_level: number
  message: string
}

interface HealthContextType {
  eegHealth: ComponentHealth | null
  mtmsDeviceHealth: ComponentHealth | null
  mepHealth: ComponentHealth | null
  remoteControllerHealth: ComponentHealth | null
  timebaseCalibratorHealth: ComponentHealth | null
}

const defaultHealthState: HealthContextType = {
  eegHealth: null,
  mtmsDeviceHealth: null,
  mepHealth: null,
  remoteControllerHealth: null,
  timebaseCalibratorHealth: null,
}

export const HealthContext = React.createContext<HealthContextType>(defaultHealthState)

interface HealthProviderProps {
  children: ReactNode
}

export const HealthProvider: React.FC<HealthProviderProps> = ({ children }) => {
  const [eegHealth, setEegHealth] = useState<ComponentHealth | null>(null)
  const [mtmsDeviceHealth, setMtmsDeviceHealth] = useState<ComponentHealth | null>(null)
  const [mepHealth, setMepHealth] = useState<ComponentHealth | null>(null)
  const [remoteControllerHealth, setRemoteControllerHealth] = useState<ComponentHealth | null>(null)
  const [timebaseCalibratorHealth, setTimebaseCalibratorHealth] = useState<ComponentHealth | null>(null)

  useEffect(() => {
    const eegSubscriber = new ROSLIB.Topic<ComponentHealth>({
      ros: ros,
      name: '/mtms/eeg/health',
      messageType: 'mtms_system_interfaces/ComponentHealth',
    })

    const mtmsSubscriber = new ROSLIB.Topic<ComponentHealth>({
      ros: ros,
      name: '/mtms/device/health',
      messageType: 'mtms_system_interfaces/ComponentHealth',
    })

    const mepSubscriber = new ROSLIB.Topic<ComponentHealth>({
      ros: ros,
      name: '/mtms/mep/health',
      messageType: 'mtms_system_interfaces/ComponentHealth',
    })

    const remoteControllerSubscriber = new ROSLIB.Topic<ComponentHealth>({
      ros: ros,
      name: '/mtms/remote_controller/health',
      messageType: 'mtms_system_interfaces/ComponentHealth',
    })

    const timebaseCalibratorSubscriber = new ROSLIB.Topic<ComponentHealth>({
      ros: ros,
      name: '/mtms/timebase_calibrator/health',
      messageType: 'mtms_system_interfaces/ComponentHealth',
    })

    eegSubscriber.subscribe((message) => {
      setEegHealth(message)
    })

    mtmsSubscriber.subscribe((message) => {
      setMtmsDeviceHealth(message)
    })

    mepSubscriber.subscribe((message) => {
      setMepHealth(message)
    })

    remoteControllerSubscriber.subscribe((message) => {
      setRemoteControllerHealth(message)
    })

    timebaseCalibratorSubscriber.subscribe((message) => {
      setTimebaseCalibratorHealth(message)
    })

    return () => {
      eegSubscriber.unsubscribe()
      mtmsSubscriber.unsubscribe()
      mepSubscriber.unsubscribe()
      remoteControllerSubscriber.unsubscribe()
      timebaseCalibratorSubscriber.unsubscribe()
    }
  }, [])

  return (
    <HealthContext.Provider
      value={{
        eegHealth,
        mtmsDeviceHealth,
        mepHealth,
        remoteControllerHealth,
        timebaseCalibratorHealth,
      }}
    >
      {children}
    </HealthContext.Provider>
  )
}
