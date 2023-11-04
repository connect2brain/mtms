import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

import {
  IndentedStateTitle,
  StyledPanel,
  StateRow,
  StateTitle,
  StateValue,
  Select,
  GrayedOutPanel,
} from 'styles/General'

import { DatasetContext } from 'providers/DatasetProvider'
import { setDatasetRos, setPlaybackRos, setLoopRos } from 'ros/ros'
import { formatTime, formatFrequency } from 'utils/utils'

const DatasetPanel = styled(StyledPanel)`
  width: 300px;
  height: 260px;
  position: fixed;
  top: 982px;
  right: 5px;
  z-index: 1000;
`

const DatasetSelect = styled(Select)`
  width: 150px;
`

const SwitchWrapper = styled.span`
  width: 95px;
`

export const DatasetDisplay: React.FC = () => {
  const { datasetList, dataset, playback, loop } = useContext(DatasetContext)

  const handleDatasetChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const newDataset = event.target.value

    setDatasetRos(newDataset, () => {
      console.log('Dataset set to ' + newDataset)
    })
  }

  const handlePlaybackChange = (playback: boolean) => {
    setPlaybackRos(playback, () => {
      console.log('Playback set to ' + playback)
    })
  }

  const handleLoopChange = (loop: boolean) => {
    setLoopRos(loop, () => {
      console.log('Loop set to ' + loop)
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
      <br />
      <StateRow>
        <StateTitle>Playback:</StateTitle>
        <SwitchWrapper>
          <ToggleSwitch type='flat' checked={playback} onChange={handlePlaybackChange} disabled={false} />
        </SwitchWrapper>
      </StateRow>
      <GrayedOutPanel isGrayedOut={!playback}>
        <StateRow>
          <IndentedStateTitle>Loop:</IndentedStateTitle>
          <SwitchWrapper>
            <ToggleSwitch type='flat' checked={loop} onChange={handleLoopChange} disabled={!playback} />
          </SwitchWrapper>
        </StateRow>
      </GrayedOutPanel>
    </DatasetPanel>
  )
}
