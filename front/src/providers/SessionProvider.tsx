import React, { useState, useEffect, ReactNode } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'

export const SessionState = {
  STOPPED: 0,
  STARTING: 1,
  STARTED: 2,
  STOPPING: 3,
}

interface Session extends ROSLIB.Message {
  state: number
  time: number
}

interface SessionContextType {
  session: Session | null
}

const defaultSessionState: SessionContextType = {
  session: null,
}

export const SessionContext = React.createContext<SessionContextType>(defaultSessionState)

interface SessionProviderProps {
  children: ReactNode
}

export const SessionProvider: React.FC<SessionProviderProps> = ({ children }) => {
  const [session, setSession] = useState<Session | null>(null)

  useEffect(() => {
    const sessionSubscriber = new ROSLIB.Topic<Session>({
      ros: ros,
      name: '/mtms/device/session',
      messageType: 'mtms_system_interfaces/Session',
    })

    let sessionTimeout: NodeJS.Timeout | null = null

    sessionSubscriber.subscribe((message) => {
      setSession(message)
      if (sessionTimeout) {
        clearTimeout(sessionTimeout)
      }
      sessionTimeout = setTimeout(() => {
        setSession(null)
      }, 1200)
    })

    return () => {
      sessionSubscriber.unsubscribe()
      if (sessionTimeout) {
        clearTimeout(sessionTimeout)
      }
    }
  }, [])

  return <SessionContext.Provider value={{ session }}>{children}</SessionContext.Provider>
}
