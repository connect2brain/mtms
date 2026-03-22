import React, { ReactNode, useEffect, useState } from 'react'
import ROSLIB from '@foxglove/roslibjs'

import { ros } from 'ros/ros'

/** Matches `mtms_experiment_interfaces/msg/ExperimentState.msg`. */
export const EXPERIMENT_STATE = {
  NOT_RUNNING: 0,
  RUNNING: 1,
  PAUSE_REQUESTED: 2,
  PAUSED: 3,
  CANCEL_REQUESTED: 4,
  STOPPING: 5,
} as const

export type ExperimentStateMessage = {
  state: number
  /* Seconds since entering PAUSED. 0.0 when not paused. */
  paused_for_seconds?: number
}

interface ExperimentContextType {
  /** Latest `/mtms/experiment/state` message, or null before the first message. */
  experimentStateMessage: ExperimentStateMessage | null
  /** Latest `/mtms/experiment/feedback` message, or null before the first message. */
  experimentFeedbackMessage: unknown | null
}

const defaultExperimentContext: ExperimentContextType = {
  experimentStateMessage: null,
  experimentFeedbackMessage: null,
}

export const ExperimentContext = React.createContext<ExperimentContextType>(defaultExperimentContext)

interface ExperimentProviderProps {
  children: ReactNode
}

export const ExperimentProvider: React.FC<ExperimentProviderProps> = ({ children }) => {
  const [experimentStateMessage, setExperimentStateMessage] = useState<ExperimentStateMessage | null>(null)
  const [experimentFeedbackMessage, setExperimentFeedbackMessage] = useState<unknown | null>(null)

  useEffect(() => {
    const stateTopic = new ROSLIB.Topic({
      ros,
      name: '/mtms/experiment/state',
      messageType: 'mtms_experiment_interfaces/ExperimentState',
    })
    const feedbackTopic = new ROSLIB.Topic({
      ros,
      name: '/mtms/experiment/feedback',
      messageType: 'mtms_experiment_interfaces/ExperimentFeedback',
    })

    const onState = (msg: any) => {
      setExperimentStateMessage(msg as ExperimentStateMessage)
    }
    const onFeedback = (msg: any) => {
      setExperimentFeedbackMessage(msg)
    }

    stateTopic.subscribe(onState)
    feedbackTopic.subscribe(onFeedback)

    return () => {
      stateTopic.unsubscribe(onState)
      feedbackTopic.unsubscribe(onFeedback)
    }
  }, [])

  return (
    <ExperimentContext.Provider value={{ experimentStateMessage, experimentFeedbackMessage }}>
      {children}
    </ExperimentContext.Provider>
  )
}
