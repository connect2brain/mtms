import React, { ReactNode, useEffect, useState } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'
import { useRosConnection } from './RosConnectionProvider'

interface RemoteControllerStartedMessage extends ROSLIB.Message {
  data: boolean
}

interface RemoteControllerContextType {
  started: boolean | null
}

const defaultRemoteControllerContext: RemoteControllerContextType = {
  started: null,
}

export const RemoteControllerContext = React.createContext<RemoteControllerContextType>(defaultRemoteControllerContext)

interface Props {
  children: ReactNode
}

export const RemoteControllerProvider: React.FC<Props> = ({ children }) => {
  const { isConnected } = useRosConnection()
  const [started, setStarted] = useState<boolean | null>(null)

  useEffect(() => {
    if (!isConnected) {
      setStarted(null)
      return
    }

    const startedSubscriber = new ROSLIB.Topic<RemoteControllerStartedMessage>({
      ros: ros,
      name: '/mtms/remote_controller/started',
      messageType: 'std_msgs/Bool',
    })

    startedSubscriber.subscribe((message) => {
      setStarted(Boolean(message.data))
    })

    return () => {
      startedSubscriber.unsubscribe()
    }
  }, [isConnected])

  return <RemoteControllerContext.Provider value={{ started }}>{children}</RemoteControllerContext.Provider>
}

