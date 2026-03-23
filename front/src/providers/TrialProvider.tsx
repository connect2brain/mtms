import React, { ReactNode, useEffect, useState } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'

const TRIAL_READINESS_TOPIC = '/mtms/trial/trial_readiness'

interface TrialReadinessMessage extends ROSLIB.Message {
  data: boolean
}

interface TrialProviderContextType {
  trialReadiness: boolean | null
}

const defaultTrialProviderContext: TrialProviderContextType = {
  trialReadiness: null,
}

export const TrialReadinessContext = React.createContext<TrialProviderContextType>(defaultTrialProviderContext)

interface TrialProviderProps {
  children: ReactNode
}

export const TrialProvider: React.FC<TrialProviderProps> = ({ children }) => {
  const [trialReadiness, setTrialReadiness] = useState<boolean | null>(null)

  useEffect(() => {
    const subscriber = new ROSLIB.Topic<TrialReadinessMessage>({
      ros,
      name: TRIAL_READINESS_TOPIC,
      messageType: 'std_msgs/Bool',
    })

    let staleTimeout: NodeJS.Timeout | null = null

    subscriber.subscribe((message) => {
      setTrialReadiness(message.data)

      // If messages stop arriving, treat the status as unknown.
      if (staleTimeout) {
        clearTimeout(staleTimeout)
      }
      staleTimeout = setTimeout(() => {
        setTrialReadiness(null)
      }, 1200)
    })

    return () => {
      subscriber.unsubscribe()
      if (staleTimeout) {
        clearTimeout(staleTimeout)
      }
    }
  }, [])

  return <TrialReadinessContext.Provider value={{ trialReadiness }}>{children}</TrialReadinessContext.Provider>
}

