import React from 'react'
import styled from 'styled-components'
import { startDevice, startExperiment, stopDevice, stopExperiment } from 'services/experiment'
import { DeviceState, DeviceStateMessage, ExperimentState, ExperimentStateMessage } from 'types/fpga'

type Props = {
  deviceState: DeviceStateMessage
  experimentState: ExperimentStateMessage
}

export const ExperimentControl = ({ deviceState, experimentState }: Props) => {
  const deviceText = () => {
    if (deviceState.value === DeviceState.NOT_OPERATIONAL) return 'Start'
    if (deviceState.value === DeviceState.STARTUP) return 'Starting'
    if (deviceState.value === DeviceState.OPERATIONAL) return 'Stop'
    if (deviceState.value === DeviceState.SHUTDOWN) return 'Stopping'
    return '???'
  }

  const experimentText = () => {
    if (experimentState.value === ExperimentState.STOPPED) return 'Start'
    if (experimentState.value === ExperimentState.STARTING) return 'Starting'
    if (experimentState.value === ExperimentState.STARTED) return 'Stop'
    if (experimentState.value === ExperimentState.STOPPING) return 'Stopping'
    return '???'
  }

  const toggleDevice = () => {
    if (deviceState.value === DeviceState.NOT_OPERATIONAL) {
      startDevice()
    } else if (deviceState.value === DeviceState.OPERATIONAL) {
      stopDevice()
    }
  }

  const toggleExperiment = () => {
    if (experimentState.value === ExperimentState.STOPPED) {
      startExperiment()
    } else if (experimentState.value === ExperimentState.STARTED) {
      stopExperiment()
    }
  }

  return (
    <div>
      <StyledButton
        onClick={toggleDevice}
        disabled={deviceState.value === DeviceState.STARTUP || deviceState.value === DeviceState.SHUTDOWN}
      >
        {deviceText()} device
      </StyledButton>
      <br />
      <StyledButton
        onClick={toggleExperiment}
        disabled={
          experimentState.value === ExperimentState.STARTING || experimentState.value === ExperimentState.STOPPING
        }
      >
        {experimentText()} experiment
      </StyledButton>
    </div>
  )
}

const StyledButton = styled.button``
