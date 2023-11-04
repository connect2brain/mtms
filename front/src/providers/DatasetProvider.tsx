import React, { useState, useEffect, ReactNode } from 'react'
import { Topic, Message } from 'roslib'

import { ros } from 'ros/ros'

interface Dataset extends ROSLIB.Message {
  name: string
  filename: string
}

interface DatasetList extends ROSLIB.Message {
  datasets: Dataset[]
}

interface DatasetContextType {
  datasetList: Dataset[]
}

const defaultDatasetState: DatasetContextType = {
  datasetList: [],
}

export const DatasetContext = React.createContext<DatasetContextType>(defaultDatasetState)

interface DatasetProviderProps {
  children: ReactNode
}

export const DatasetProvider: React.FC<DatasetProviderProps> = ({ children }) => {
  const [datasetList, setDatasetList] = useState<Dataset[]>([])

  useEffect(() => {
    /* Subscriber for dataset list. */
    const datasetListSubscriber = new Topic<DatasetList>({
      ros: ros,
      name: '/eeg_simulator/dataset/list',
      messageType: 'project_interfaces/DatasetList',
    })

    datasetListSubscriber.subscribe((message) => {
      console.log(message)
      setDatasetList(message.datasets)
    })

    /* Unsubscribers */
    return () => {
      datasetListSubscriber.unsubscribe()
    }
  }, [])

  return (
    <DatasetContext.Provider
      value={{
        datasetList,
      }}
    >
      {children}
    </DatasetContext.Provider>
  )
}
