import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { StyledPanel, StateRow, StateTitle, StateValue, Select } from 'styles/General'
import { DatasetContext } from 'providers/DatasetProvider'
import { setDatasetRos } from 'ros/ros'

const DatasetPanel = styled(StyledPanel)`
  width: 300px;
  height: 150px;
  position: fixed;
  top: 982px;
  right: 5px;
  z-index: 1000;
`

const DatasetSelect = styled(Select)`
  width: 150px;
`

export const DatasetDisplay: React.FC = () => {
  const { datasetList, dataset } = useContext(DatasetContext)

  const handleDatasetChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const newDataset = event.target.value

    setDatasetRos(newDataset, () => {
      console.log('Dataset set to ' + newDataset)
    })
  }

  return (
    <DatasetPanel>
      <StateRow>
        <StateTitle>Dataset:</StateTitle>
        <StateValue>
          <DatasetSelect onChange={handleDatasetChange} value={dataset}>
            {datasetList.map((dataset, index) => (
              <option key={index} value={dataset.filename}>
                {dataset.name}
              </option>
            ))}
          </DatasetSelect>
        </StateValue>
      </StateRow>
      <br />
    </DatasetPanel>
  )
}
