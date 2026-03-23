import React, { useContext, useEffect, useMemo, useState } from 'react'
import styled from 'styled-components'

import { StyledButton, StyledRedButton, StyledPanel } from 'styles/General'
import { SmallerTitle } from 'styles/ExperimentStyles'
import { ElectricTarget, startRemoteController, stopRemoteController } from 'ros/remote_controller'

import { SystemContext, DeviceState } from 'providers/SystemProvider'
import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { RemoteControllerContext, RemoteControllerState } from 'providers/RemoteControllerProvider'

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

interface RemoteControlProps {
  /**
   * Called before starting; should validate the pulse table, show any errors, and return
   * the target lists to cache, or null to abort the start.
   */
  getTargetLists?: () => ElectricTarget[][] | null
}

export const RemoteControl = ({ getTargetLists }: RemoteControlProps) => {
  const { systemState } = useContext(SystemContext)
  const { mtmsDeviceHealthcheck } = useContext(HealthcheckContext)
  const { state: remoteControllerState } = useContext(RemoteControllerContext)

  const isDeviceOperational = systemState?.device_state.value === DeviceState.OPERATIONAL

  // Debounce visual updates so the button "inertia" doesn't flicker when state changes.
  const [displayRemoteControllerState, setDisplayRemoteControllerState] = useState(remoteControllerState)
  useEffect(() => {
    const timeoutId = window.setTimeout(() => {
      setDisplayRemoteControllerState(remoteControllerState)
    }, 200)

    return () => window.clearTimeout(timeoutId)
  }, [remoteControllerState])

  const label = useMemo(() => {
    if (displayRemoteControllerState === null) return 'Waiting...'
    if (displayRemoteControllerState === RemoteControllerState.CACHING) return 'Caching...'
    if (displayRemoteControllerState === RemoteControllerState.STARTED) return 'Stop'
    return 'Start'
  }, [displayRemoteControllerState])

  const onToggle = () => {
    if (remoteControllerState === RemoteControllerState.STARTED) {
      stopRemoteController()
    } else {
      if (remoteControllerState === RemoteControllerState.CACHING) return
      if (getTargetLists) {
        const targetLists = getTargetLists()
        if (targetLists === null) return
        startRemoteController(targetLists)
      } else {
        startRemoteController([])
      }
    }
  }

  if (mtmsDeviceHealthcheck?.status === HealthcheckStatus.DISABLED) {
    return null
  }

  // Use debounced state for button color/text selection.
  const isWaitingOrCaching =
    displayRemoteControllerState === null || displayRemoteControllerState === RemoteControllerState.CACHING
  const isStarted = displayRemoteControllerState === RemoteControllerState.STARTED
  const isDisabled = isWaitingOrCaching || !isDeviceOperational

  return (
    <RemoteControlPanel>
      <RemoteControlTitle>Remote control</RemoteControlTitle>
      {isStarted ? (
        <StyledRedButton onClick={onToggle} disabled={isDisabled}>
          {label}
        </StyledRedButton>
      ) : (
        <StyledButton onClick={isWaitingOrCaching ? undefined : onToggle} disabled={isDisabled}>
          {label}
        </StyledButton>
      )}
    </RemoteControlPanel>
  )
}

