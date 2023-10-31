import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'ros/ros'

interface Latency extends ROSLIB.Message {
  latency: number
  sample_time: number
}

interface PreprocessorList extends ROSLIB.Message {
  scripts: string[]
}

interface DeciderList extends ROSLIB.Message {
  scripts: string[]
}

interface RosBoolean extends ROSLIB.Message {
  data: boolean
}

interface PipelineContextType {
  preprocessorList: string[]
  preprocessorEnabled: boolean

  deciderList: string[]
  deciderEnabled: boolean

  latency: Latency | null
  setLatency: React.Dispatch<React.SetStateAction<Latency | null>>
}

const defaultPipelineState: PipelineContextType = {
  preprocessorList: [],
  preprocessorEnabled: false,

  deciderList: [],
  deciderEnabled: false,

  latency: null,
  setLatency: () => {
    console.warn('setLatency is not yet initialized.')
  },
}

export const PipelineContext = React.createContext<PipelineContextType>(defaultPipelineState)

interface PipelineProviderProps {
  children: ReactNode
}

export const PipelineProvider: React.FC<PipelineProviderProps> = ({ children }) => {
  const [preprocessorList, setPreprocessorList] = useState<string[]>([])
  const [preprocessorEnabled, setPreprocessorEnabled] = useState<boolean>(false)

  const [deciderList, setDeciderList] = useState<string[]>([])
  const [deciderEnabled, setDeciderEnabled] = useState<boolean>(false)

  const [latency, setLatency] = useState<Latency | null>(null)

  useEffect(() => {
    /* Subscriber for preprocessor list. */
    const preprocessorListSubscriber = new Topic<PreprocessorList>({
      ros: ros,
      name: '/pipeline/preprocessor/list',
      messageType: 'project_interfaces/PreprocessorList',
    })

    preprocessorListSubscriber.subscribe((message) => {
      setPreprocessorList(message.scripts)
    })

    /* Subscriber for preprocessor enabled. */
    const preprocessorEnabledSubscriber = new Topic<RosBoolean>({
      ros: ros,
      name: '/pipeline/preprocessor/enabled',
      messageType: 'std_msgs/Bool',
    })

    preprocessorEnabledSubscriber.subscribe((message) => {
      setPreprocessorEnabled(message.data)
    })

    /* Subscriber for decider list. */
    const deciderListSubscriber = new Topic<DeciderList>({
      ros: ros,
      name: '/pipeline/decider/list',
      messageType: 'project_interfaces/DeciderList',
    })

    deciderListSubscriber.subscribe((message) => {
      setDeciderList(message.scripts)
    })

    /* Subscriber for decider enabled. */
    const deciderEnabledSubscriber = new Topic<RosBoolean>({
      ros: ros,
      name: '/pipeline/decider/enabled',
      messageType: 'std_msgs/Bool',
    })

    deciderEnabledSubscriber.subscribe((message) => {
      setDeciderEnabled(message.data)
    })

    /* Subscriber for latency. */
    const latencySubscriber = new Topic<Latency>({
      ros: ros,
      name: '/pipeline/latency',
      messageType: 'pipeline_interfaces/Latency',
    })

    latencySubscriber.subscribe((message) => {
      console.log(message)
      console.log('somo')
      setLatency(message)
    })

    /* Unsubscribers */
    return () => {
      preprocessorListSubscriber.unsubscribe()
      preprocessorEnabledSubscriber.unsubscribe()

      deciderListSubscriber.unsubscribe()
      deciderEnabledSubscriber.unsubscribe()

      latencySubscriber.unsubscribe()
    }
  }, [])

  return (
    <PipelineContext.Provider
      value={{ preprocessorList, preprocessorEnabled, deciderList, deciderEnabled, latency, setLatency }}
    >
      {children}
    </PipelineContext.Provider>
  )
}
