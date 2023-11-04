import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { IndentedStateTitle, StyledPanel, StateRow, StateTitle, StateValue, Select } from 'styles/General'
import { DatasetContext } from 'providers/DatasetProvider'
import { setDatasetRos } from 'ros/ros'
import { formatTime, formatFrequency } from 'utils/utils'

const DatasetPanel = styled(StyledPanel)`
  width: 300px;
  height: 180px;
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
  const selectedDataset = datasetList.find((d) => d.filename === dataset)

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
      <StateRow>
        <StateTitle>Sampling rate:</StateTitle>
        <StateValue>{formatFrequency(selectedDataset?.sampling_frequency)}</StateValue>
      </StateRow>
      <StateRow>
        <StateTitle>Channels</StateTitle>
      </StateRow>
      <StateRow>
        <IndentedStateTitle>EEG</IndentedStateTitle>
        <StateValue>{selectedDataset?.num_of_eeg_channels}</StateValue>
      </StateRow>
      <StateRow>
        <IndentedStateTitle>EMG</IndentedStateTitle>
        <StateValue>{selectedDataset?.num_of_emg_channels}</StateValue>
      </StateRow>
      <StateRow>
        <IndentedStateTitle>Duration</IndentedStateTitle>
        <StateValue>{formatTime(selectedDataset?.duration)}</StateValue>
      </StateRow>
    </DatasetPanel>
  )
}
