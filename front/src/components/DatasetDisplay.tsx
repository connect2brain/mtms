import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { StyledPanel, StateRow, StateTitle, StateValue, Select } from 'styles/General'
import { getKeyByValue } from 'utils'

import { DatasetContext } from 'providers/DatasetProvider'

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
  const { datasetList } = useContext(DatasetContext)

  const handleDatasetChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const newDataset = event.target.value
    /*
    setDatasetRos(newDataset, () => {
      console.log('Dataset set to ' + newDataset)
    })
    */
  }

  return (
    <DatasetPanel>
      <StateRow>
        <StateTitle>Dataset:</StateTitle>
        <StateValue>
          <DatasetSelect onChange={handleDatasetChange} value={''}>
            {datasetList.map((dataset, index) => (
              <option key={index} value={dataset.name}>
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
