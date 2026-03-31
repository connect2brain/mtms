import React, { useContext, useEffect, useMemo, useState } from 'react'
import styled from 'styled-components'

import { StyledButton, StyledRedButton, StyledPanel } from 'styles/General'
import { SmallerTitle } from 'styles/ExperimentStyles'
import { ElectricTarget, startRemoteController, stopRemoteController } from 'ros/remote_controller'

import { SystemContext, DeviceState, SessionState } from 'providers/SystemProvider'
import { RemoteControllerContext, RemoteControllerState } from 'providers/RemoteControllerProvider'
import { EegDeviceInfoContext } from 'providers/EegDeviceInfoProvider'

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
  rowsDefined?: boolean
}

export const RemoteControl = ({ getTargetLists, rowsDefined = false }: RemoteControlProps) => {
  const { systemState, session } = useContext(SystemContext)
  const { state: remoteControllerState } = useContext(RemoteControllerContext)
  const { eegDeviceInfo } = useContext(EegDeviceInfoContext)

  const isDeviceOperational = systemState?.device_state.value === DeviceState.OPERATIONAL
  const isStreaming = Boolean(eegDeviceInfo?.is_streaming)
  const isSessionOngoing = session !== null && session.state !== SessionState.STOPPED

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
    /* Do not expose to the user the distinction between CACHING and STARTING. It's an internal state. */
    if (displayRemoteControllerState === RemoteControllerState.CACHING) return 'Starting...'
    if (displayRemoteControllerState === RemoteControllerState.STARTING) return 'Starting...'
    if (displayRemoteControllerState === RemoteControllerState.STARTED) return 'Stop'
    if (displayRemoteControllerState === RemoteControllerState.STOPPING) return 'Stopping...'
    return 'Start'
  }, [displayRemoteControllerState])

  const onToggle = () => {
    if (remoteControllerState === RemoteControllerState.STARTED) {
      stopRemoteController()
      return
    }
    if (remoteControllerState === RemoteControllerState.CACHING) return
    if (remoteControllerState === RemoteControllerState.STOPPING) return
    if (remoteControllerState === RemoteControllerState.STARTING) return
    if (getTargetLists) {
      const targetLists = getTargetLists()
      if (targetLists === null) return
      startRemoteController(targetLists)
    } else {
      startRemoteController([])
    }
  }

  // Use debounced state for button color/text selection.
  const isWaitingOrCachingOrStarting =
    displayRemoteControllerState === null || displayRemoteControllerState === RemoteControllerState.CACHING || displayRemoteControllerState === RemoteControllerState.STARTING
  const isStopping = displayRemoteControllerState === RemoteControllerState.STOPPING
  const isStarted = displayRemoteControllerState === RemoteControllerState.STARTED
  const readyToStart = rowsDefined && isStreaming && !isSessionOngoing
  const isDisabled = isWaitingOrCachingOrStarting || isStopping || !isDeviceOperational || (!isStarted && !readyToStart)

  return (
    <RemoteControlPanel>
      <RemoteControlTitle>Remote control</RemoteControlTitle>
      {isStarted ? (
        <StyledRedButton onClick={onToggle} disabled={isDisabled}>
          {label}
        </StyledRedButton>
      ) : (
        <StyledButton onClick={isWaitingOrCachingOrStarting ? undefined : onToggle} disabled={isDisabled}>
          {label}
        </StyledButton>
      )}
    </RemoteControlPanel>
  )
}

