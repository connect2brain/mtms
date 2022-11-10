import React from 'react'
import styled from 'styled-components'
import { startDevice, startExperiment, stopDevice, stopExperiment } from 'services/experiment'
import { DeviceStateMessage, ExperimentStateMessage } from 'types/fpga'

type Props = {
  deviceState: DeviceStateMessage
  experimentState: ExperimentStateMessage
}

export const ExperimentControl = ({ deviceState, experimentState }: Props) => {
  const deviceText = () => {
    if (deviceState.value === deviceState.NOT_OPERATIONAL) return 'Start'
    if (deviceState.value === deviceState.STARTUP) return 'Starting'
    if (deviceState.value === deviceState.OPERATIONAL) return 'Stop'
    if (deviceState.value === deviceState.SHUTDOWN) return 'Stopping'
    return '???'
  }

  const experimentText = () => {
    if (experimentState.value === experimentState.STOPPED) return 'Start'
    if (experimentState.value === experimentState.STARTING) return 'Starting'
    if (experimentState.value === experimentState.STARTED) return 'Stop'
    if (experimentState.value === experimentState.STOPPING) return 'Stopping'
    return '???'
  }

  const toggleDevice = () => {
    if (deviceState.value === deviceState.NOT_OPERATIONAL) {
      startDevice()
    } else if (deviceState.value === deviceState.OPERATIONAL) {
      stopDevice()
    }
  }

  const toggleExperiment = () => {
    if (experimentState.value === experimentState.STOPPED) {
      startExperiment()
    } else if (experimentState.value === experimentState.STARTED) {
      stopExperiment()
    }
  }

  return (
    <div>
      <StyledButton
        onClick={toggleDevice}
        disabled={deviceState.value === deviceState.STARTUP || deviceState.value === deviceState.SHUTDOWN}
      >
        {deviceText()} device
      </StyledButton>
      <br />
      <StyledButton
        onClick={toggleExperiment}
        disabled={
          experimentState.value === experimentState.STARTING || experimentState.value === experimentState.STOPPING
        }
      >
        {experimentText()} experiment
      </StyledButton>
    </div>
  )
}

const StyledButton = styled.button``
