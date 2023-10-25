import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'services/ros'

interface PreprocessorList extends ROSLIB.Message {
  scripts: string[],
}

interface RosBoolean extends ROSLIB.Message {
  data: boolean,
}

interface PipelineContextType {
  preprocessorList: string[]
  preprocessorEnabled: boolean
}

const defaultPipelineState: PipelineContextType = {
  preprocessorList: [],
  preprocessorEnabled: false
}

export const PipelineContext = React.createContext<PipelineContextType>(defaultPipelineState)

interface PipelineProviderProps {
  children: ReactNode
}

export const PipelineProvider: React.FC<PipelineProviderProps> = ({ children }) => {
  const [preprocessorList, setPreprocessorList] = useState<string[]>([])
  const [preprocessorEnabled, setPreprocessorEnabled] = useState<boolean>(false)

  useEffect(() => {
    /* Subscriber for preprocessor list. */
    const preprocessorListSubscriber = new Topic({
      ros: ros,
      name: '/pipeline/preprocessor/list',
      messageType: 'project_interfaces/PreprocessorList'
    }) as Topic<PreprocessorList>

    preprocessorListSubscriber.subscribe((message) => {
      setPreprocessorList(message.scripts)
    })

    /* Subscriber for preprocessor enabled. */
    const preprocessorEnabledSubscriber = new Topic({
      ros: ros,
      name: '/pipeline/preprocessor/enabled',
      messageType: 'std_msgs/Bool'
    }) as Topic<RosBoolean>

    preprocessorEnabledSubscriber.subscribe((message) => {
      setPreprocessorEnabled(message.data)
    })

    /* Unsubscribers */
    return () => {
      preprocessorListSubscriber.unsubscribe()
      preprocessorEnabledSubscriber.unsubscribe()
    }
  }, [])

  return (
    <PipelineContext.Provider value={{ preprocessorList, preprocessorEnabled }}>
      {children}
    </PipelineContext.Provider>
  )
}
