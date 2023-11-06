import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { StyledPanel, StateRow, StateTitle, IndentedStateTitle, StateValue } from 'styles/General'

import { EegContext } from 'providers/EegProvider'

const EegStatisticsPanelTitle = styled.div`
  width: 340px;
  position: fixed;
  top: 896px;
  right: 5px;
  z-index: 1001;
  text-align: left;
  font-size: 20px;
  font-weight: bold;
`

const EegStatisticsPanel = styled(StyledPanel)`
  width: 300px;
  height: 250px;
  position: fixed;
  top: 928px;
  right: 5px;
  z-index: 1000;
`

export const EegStatisticsDisplay: React.FC = () => {
  const { eegStatistics } = useContext(EegContext)

  const formatTimeToMicroseconds = (timeInSeconds?: number): string | undefined => {
    if (typeof timeInSeconds === 'undefined' || timeInSeconds === null) {
      return undefined
    }
    return `${(timeInSeconds * 1_000_000).toFixed(0)}`
  }

  return (
    <>
      <EegStatisticsPanelTitle>Statistics</EegStatisticsPanelTitle>
      <EegStatisticsPanel>
        <StateRow>
          <StateTitle>Raw</StateTitle>
          <StateValue>{eegStatistics?.num_of_raw_samples}</StateValue>
        </StateRow>
        <StateRow>
          <StateTitle>Preprocessed</StateTitle>
          <StateValue>{eegStatistics?.num_of_preprocessed_samples}</StateValue>
        </StateRow>
        <br />
        <StateRow>
          <StateTitle>Processing time (µs)</StateTitle>
        </StateRow>
        <StateRow>
          <IndentedStateTitle>Median</IndentedStateTitle>
          <StateValue>{formatTimeToMicroseconds(eegStatistics?.preprocessing_time_median)}</StateValue>
        </StateRow>
        <StateRow>
          <IndentedStateTitle>Q95</IndentedStateTitle>
          <StateValue>{formatTimeToMicroseconds(eegStatistics?.preprocessing_time_q95)}</StateValue>
        </StateRow>
        <StateRow>
          <IndentedStateTitle>Max</IndentedStateTitle>
          <StateValue>{formatTimeToMicroseconds(eegStatistics?.preprocessing_time_max)}</StateValue>
        </StateRow>
        <br />
        <StateRow>
          <StateTitle>Max sample interval (µs)</StateTitle>
        </StateRow>
        <StateRow>
          <IndentedStateTitle>Raw</IndentedStateTitle>
          <StateValue>{formatTimeToMicroseconds(eegStatistics?.max_time_between_raw_samples)}</StateValue>
        </StateRow>
        <StateRow>
          <IndentedStateTitle>Preprocessed</IndentedStateTitle>
          <StateValue>{formatTimeToMicroseconds(eegStatistics?.max_time_between_preprocessed_samples)}</StateValue>
        </StateRow>
      </EegStatisticsPanel>
    </>
  )
}
