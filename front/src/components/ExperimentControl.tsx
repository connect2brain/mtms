import React, { useState } from 'react'
import styled from 'styled-components'
import { startDevice, startExperiment, stopDevice, stopExperiment } from '../services/experiment'

export const ExperimentControl = () => {
  const [deviceStarted, setDeviceStarted] = useState<boolean>(false)
  const [experimentStarted, setExperimentStarted] = useState<boolean>(false)

  const toggleExperiment = () => {
    if (!experimentStarted) {
      startExperiment()
    } else {
      stopExperiment()
    }
    setExperimentStarted(!experimentStarted)
  }

  const toggleDevice = () => {
    if (!deviceStarted) {
      startDevice()
    } else {
      stopDevice()
    }
    setDeviceStarted(!deviceStarted)
  }

  return (
    <div>
      <StyledButton onClick={toggleDevice}>{deviceStarted ? 'Stop' : 'Start'} device</StyledButton>
      <StyledButton onClick={toggleExperiment}>{experimentStarted ? 'Stop' : 'Start'} experiment</StyledButton>
    </div>
  )
}

const StyledButton = styled.button``
