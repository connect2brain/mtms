import React, { useState, useEffect, ReactNode } from 'react'
import { Topic } from 'roslib'
import { ros } from 'ros/ros'

interface NeuroSimoContextType {
  deciderEnabled: boolean | null
}

const defaultState: NeuroSimoContextType = {
  deciderEnabled: null,
}

export const NeuroSimoContext = React.createContext<NeuroSimoContextType>(defaultState)

interface NeuroSimoProviderProps {
  children: ReactNode
}

export const NeuroSimoProvider: React.FC<NeuroSimoProviderProps> = ({ children }) => {
  const [deciderEnabled, setDeciderEnabled] = useState<boolean | null>(null)

  useEffect(() => {
    const deciderSubscriber = new Topic<{ data: boolean }>({
      ros: ros,
      name: '/pipeline/decider/enabled',
      messageType: 'std_msgs/Bool',
    })

    deciderSubscriber.subscribe((message) => {
      setDeciderEnabled(message.data)
    })

    return () => {
      deciderSubscriber.unsubscribe()
    }
  }, [])

  return (
    <NeuroSimoContext.Provider value={{ deciderEnabled }}>
      {children}
    </NeuroSimoContext.Provider>
  )
}
