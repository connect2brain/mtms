import React, { useState } from 'react'
import styled from 'styled-components'
import {
  startDevice,
  startExperiment,
  stopDevice,
  stopExperiment,
} from '../services/experiment'

type Props = {
  deviceState: number
  experimentState: number
}

export const ExperimentControl = ({ deviceState, experimentState }: Props) => {
  const deviceText = () => {
    if (deviceState == 0) return 'Start'
    if (deviceState == 1) return 'Starting'
    if (deviceState == 2) return 'Stop'
    if (deviceState == 3) return 'Stopping'
    return '???'
  }

  const experimentText = () => {
    if (experimentState == 0) return 'Start'
    if (experimentState == 1) return 'Starting'
    if (experimentState == 2) return 'Stop'
    if (experimentState == 3) return 'Stopping'
    return '???'
  }

  const toggleDevice = () => {
    if (deviceState == 0) {
      startDevice()
    } else if (deviceState == 2) {
      stopDevice()
    }
  }

  const toggleExperiment = () => {
    if (experimentState == 0) {
      startExperiment()
    } else if (experimentState == 2) {
      stopExperiment()
    }
  }

  return (
    <div>
      <StyledButton
        onClick={toggleDevice}
        disabled={deviceState == 1 || deviceState == 3}
      >
        {deviceText()} device
      </StyledButton>
      <br />
      <StyledButton
        onClick={toggleExperiment}
        disabled={experimentState == 1 || experimentState == 3}
      >
        {experimentText()} experiment
      </StyledButton>
    </div>
  )
}

const StyledButton = styled.button``
