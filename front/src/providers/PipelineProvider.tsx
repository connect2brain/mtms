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

interface PresenterList extends ROSLIB.Message {
  scripts: string[]
}

interface RosBoolean extends ROSLIB.Message {
  data: boolean
}

interface RosString extends ROSLIB.Message {
  data: string
}

interface PipelineContextType {
  preprocessorList: string[]
  preprocessorModule: string
  preprocessorEnabled: boolean

  deciderList: string[]
  deciderModule: string
  deciderEnabled: boolean

  presenterList: string[]
  presenterModule: string
  presenterEnabled: boolean

  latency: Latency | null
  setLatency: React.Dispatch<React.SetStateAction<Latency | null>>
}

const defaultPipelineState: PipelineContextType = {
  preprocessorList: [],
  preprocessorModule: '',
  preprocessorEnabled: false,

  deciderList: [],
  deciderModule: '',
  deciderEnabled: false,

  presenterList: [],
  presenterModule: '',
  presenterEnabled: false,

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
  const [preprocessorModule, setPreprocessorModule] = useState<string>('')
  const [preprocessorEnabled, setPreprocessorEnabled] = useState<boolean>(false)

  const [deciderList, setDeciderList] = useState<string[]>([])
  const [deciderModule, setDeciderModule] = useState<string>('')
  const [deciderEnabled, setDeciderEnabled] = useState<boolean>(false)

  const [presenterList, setPresenterList] = useState<string[]>([])
  const [presenterModule, setPresenterModule] = useState<string>('')
  const [presenterEnabled, setPresenterEnabled] = useState<boolean>(false)

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

    /* Subscriber for preprocessor module. */
    const preprocessorModuleSubscriber = new Topic<RosString>({
      ros: ros,
      name: '/pipeline/preprocessor/module',
      messageType: 'std_msgs/String',
    })

    preprocessorModuleSubscriber.subscribe((message) => {
      setPreprocessorModule(message.data)
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

    /* Subscriber for decider module. */
    const deciderModuleSubscriber = new Topic<RosString>({
      ros: ros,
      name: '/pipeline/decider/module',
      messageType: 'std_msgs/String',
    })

    deciderModuleSubscriber.subscribe((message) => {
      setDeciderModule(message.data)
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

    /* Subscriber for presenter list. */
    const presenterListSubscriber = new Topic<PresenterList>({
      ros: ros,
      name: '/pipeline/presenter/list',
      messageType: 'project_interfaces/PresenterList',
    })

    presenterListSubscriber.subscribe((message) => {
      setPresenterList(message.scripts)
    })

    /* Subscriber for presenter module. */
    const presenterModuleSubscriber = new Topic<RosString>({
      ros: ros,
      name: '/pipeline/presenter/module',
      messageType: 'std_msgs/String',
    })

    presenterModuleSubscriber.subscribe((message) => {
      setPresenterModule(message.data)
    })

    /* Subscriber for presenter enabled. */
    const presenterEnabledSubscriber = new Topic<RosBoolean>({
      ros: ros,
      name: '/pipeline/presenter/enabled',
      messageType: 'std_msgs/Bool',
    })

    presenterEnabledSubscriber.subscribe((message) => {
      setPresenterEnabled(message.data)
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
      preprocessorModuleSubscriber.unsubscribe()
      preprocessorEnabledSubscriber.unsubscribe()

      deciderListSubscriber.unsubscribe()
      deciderModuleSubscriber.unsubscribe()
      deciderEnabledSubscriber.unsubscribe()

      presenterListSubscriber.unsubscribe()
      presenterModuleSubscriber.unsubscribe()
      presenterEnabledSubscriber.unsubscribe()

      latencySubscriber.unsubscribe()
    }
  }, [])

  return (
    <PipelineContext.Provider
      value={{
        preprocessorList,
        preprocessorModule,
        preprocessorEnabled,
        deciderList,
        deciderModule,
        deciderEnabled,
        presenterList,
        presenterModule,
        presenterEnabled,
        latency,
        setLatency,
      }}
    >
      {children}
    </PipelineContext.Provider>
  )
}
