import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'services/ros'

interface Preprocessors extends ROSLIB.Message {
  scripts: string[],
}

interface PipelineContextType {
  preprocessors: string[]
}

const defaultPipelineState: PipelineContextType = {
  preprocessors: []
}

export const PipelineContext = React.createContext<PipelineContextType>(defaultPipelineState)

interface PipelineProviderProps {
  children: ReactNode
}

export const PipelineProvider: React.FC<PipelineProviderProps> = ({ children }) => {
  const [preprocessors, setPreprocessors] = useState<string[]>([])

  useEffect(() => {
    const preprocessorsSubscriber = new Topic({
      ros: ros,
      name: '/pipeline/preprocessor/list',
      messageType: 'project_interfaces/PreprocessorList'
    }) as Topic<Preprocessors>

    preprocessorsSubscriber.subscribe((message) => {
      setPreprocessors(message.scripts)
    })

    return () => {
      preprocessorsSubscriber.unsubscribe()
    }
  }, [])

  return (
    <PipelineContext.Provider value={{ preprocessors }}>
      {children}
    </PipelineContext.Provider>
  )
}
