import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { StyledPanel, StateRow, StateTitle, IndentedStateTitle, StateValue } from 'styles/General'

import { EegContext } from 'providers/EegProvider'
import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'

import { formatFrequency } from 'utils/utils'

const EegPanel = styled(StyledPanel)`
  width: 300px;
  height: 105px;
  position: fixed;
  top: 240px;
  right: 5px;
  z-index: 1000;
`

export const EegDisplay: React.FC = () => {
  const { eegInfo } = useContext(EegContext)
  const { eegHealthcheck } = useContext(HealthcheckContext)

  const [eegHealthcheckOk, setEegHealthcheckOk] = useState(false)

  /* Update EEG healthcheck ok status. */
  useEffect(() => {
    setEegHealthcheckOk(eegHealthcheck?.status.value === HealthcheckStatus.READY)
  }, [eegHealthcheck])

  return (
    <EegPanel isGrayedOut={!eegHealthcheckOk}>
      <StateRow>
        <StateTitle>Sampling rate</StateTitle>
        <StateValue>{formatFrequency(eegInfo?.sampling_frequency)}</StateValue>
      </StateRow>
      <br />
      <StateRow>
        <StateTitle>Channels</StateTitle>
      </StateRow>
      <StateRow>
        <IndentedStateTitle>EEG</IndentedStateTitle>
        <StateValue>{eegInfo?.num_of_eeg_channels}</StateValue>
      </StateRow>
      <StateRow>
        <IndentedStateTitle>EMG</IndentedStateTitle>
        <StateValue>{eegInfo?.num_of_emg_channels}</StateValue>
      </StateRow>
    </EegPanel>
  )
}
