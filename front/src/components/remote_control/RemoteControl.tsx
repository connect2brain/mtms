import React, { useContext } from 'react'
import styled from 'styled-components'

import { StyledButton, StyledRedButton, StyledPanel } from 'styles/General'
import { SmallerTitle } from 'styles/ExperimentStyles'
import { startRemoteController, stopRemoteController } from 'ros/remote_controller'

import { SystemContext, DeviceState } from 'providers/SystemProvider'
import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { RemoteControllerContext } from 'providers/RemoteControllerProvider'

const RemoteControlPanel = styled(StyledPanel)`
  width: 280px;
  height: 120px;
  padding: 25px 0px 40px 0px;

  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;

  button {
    margin-top: 20px;
    margin-bottom: 0;
  }
`

const RemoteControlTitle = styled(SmallerTitle)`
  margin-bottom: 15px;
  text-align: center;
  width: 100%;
  margin-right: 0;
`

export const RemoteControl = () => {
  const { systemState } = useContext(SystemContext)
  const { mtmsDeviceHealthcheck } = useContext(HealthcheckContext)
  const { started: remoteControllerStarted } = useContext(RemoteControllerContext)

  const isDeviceOperational = systemState?.device_state.value === DeviceState.OPERATIONAL

  const onToggle = () => {
    if (remoteControllerStarted) {
      stopRemoteController()
    } else {
      startRemoteController()
    }
  }

  if (mtmsDeviceHealthcheck?.status === HealthcheckStatus.DISABLED) {
    return null
  }

  return (
    <RemoteControlPanel>
      <RemoteControlTitle>Remote control</RemoteControlTitle>
      {remoteControllerStarted === null ? (
        <StyledButton disabled={true}>Waiting...</StyledButton>
      ) : remoteControllerStarted ? (
        <StyledRedButton onClick={onToggle} disabled={!isDeviceOperational}>
          Stop
        </StyledRedButton>
      ) : (
        <StyledButton onClick={onToggle} disabled={!isDeviceOperational}>
          Start
        </StyledButton>
      )}
    </RemoteControlPanel>
  )
}

