import React, { ReactNode, useEffect, useState } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'
import { useRosConnection } from './RosConnectionProvider'

export const RemoteControllerState = {
  NOT_STARTED: 0,
  CACHING: 1,
  STARTED: 2,
  STOPPING: 3,
} as const

interface RemoteControllerStateMessage extends ROSLIB.Message {
  state: number
}

interface RemoteControllerContextType {
  state: number | null
}

const defaultRemoteControllerContext: RemoteControllerContextType = {
  state: null,
}

export const RemoteControllerContext = React.createContext<RemoteControllerContextType>(defaultRemoteControllerContext)

interface Props {
  children: ReactNode
}

export const RemoteControllerProvider: React.FC<Props> = ({ children }) => {
  const { isConnected } = useRosConnection()
  const [state, setState] = useState<number | null>(null)

  useEffect(() => {
    if (!isConnected) {
      setState(null)
      return
    }

    const stateSubscriber = new ROSLIB.Topic<RemoteControllerStateMessage>({
      ros: ros,
      name: '/mtms/remote_controller/state',
      messageType: 'mtms_trial_interfaces/msg/RemoteControllerState',
    })

    stateSubscriber.subscribe((message) => {
      setState(message.state)
    })

    return () => {
      stateSubscriber.unsubscribe()
    }
  }, [isConnected])

  return <RemoteControllerContext.Provider value={{ state }}>{children}</RemoteControllerContext.Provider>
}

