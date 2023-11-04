import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'ros/ros'

interface DatasetChannels {
  eeg: number
  emg: number
}

interface Dataset extends ROSLIB.Message {
  name: string
  filename: string
  sampling_frequency: number
  channels: DatasetChannels
}

interface DatasetList extends ROSLIB.Message {
  datasets: Dataset[]
}

interface RosString extends ROSLIB.Message {
  data: string
}

interface DatasetContextType {
  datasetList: Dataset[]
  dataset: string
}

const defaultDatasetState: DatasetContextType = {
  datasetList: [],
  dataset: '',
}

export const DatasetContext = React.createContext<DatasetContextType>(defaultDatasetState)

interface DatasetProviderProps {
  children: ReactNode
}

export const DatasetProvider: React.FC<DatasetProviderProps> = ({ children }) => {
  const [datasetList, setDatasetList] = useState<Dataset[]>([])
  const [dataset, setDataset] = useState<string>('')

  useEffect(() => {
    /* Subscriber for dataset list. */
    const datasetListSubscriber = new Topic<DatasetList>({
      ros: ros,
      name: '/eeg_simulator/dataset/list',
      messageType: 'project_interfaces/DatasetList',
    })

    datasetListSubscriber.subscribe((message) => {
      setDatasetList(message.datasets)
    })

    /* Subscriber for active dataset. */
    const datasetSubscriber = new Topic<RosString>({
      ros: ros,
      name: '/eeg_simulator/dataset',
      messageType: 'std_msgs/String',
    })

    datasetSubscriber.subscribe((message) => {
      setDataset(message.data)
    })

    /* Unsubscribers */
    return () => {
      datasetListSubscriber.unsubscribe()
      datasetSubscriber.unsubscribe()
    }
  }, [])

  return (
    <DatasetContext.Provider
      value={{
        datasetList,
        dataset,
      }}
    >
      {children}
    </DatasetContext.Provider>
  )
}
