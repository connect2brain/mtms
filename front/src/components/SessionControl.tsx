import React from 'react'
import styled from 'styled-components'
import { startDevice, startSession, stopDevice, stopSession } from 'services/session'
import { DeviceState, DeviceStateMessage, SessionState, SessionStateMessage } from 'types/mtmsDevice'

type Props = {
  deviceState: DeviceStateMessage
  sessionState: SessionStateMessage
}

export const SessionControl = ({ deviceState, sessionState }: Props) => {
  const deviceText = () => {
    if (deviceState.value === DeviceState.NOT_OPERATIONAL) return 'Start'
    if (deviceState.value === DeviceState.STARTUP) return 'Starting'
    if (deviceState.value === DeviceState.OPERATIONAL) return 'Stop'
    if (deviceState.value === DeviceState.SHUTDOWN) return 'Stopping'
    return '???'
  }

  const sessionText = () => {
    if (sessionState.value === SessionState.STOPPED) return 'Start'
    if (sessionState.value === SessionState.STARTING) return 'Starting'
    if (sessionState.value === SessionState.STARTED) return 'Stop'
    if (sessionState.value === SessionState.STOPPING) return 'Stopping'
    return '???'
  }

  const toggleDevice = () => {
    if (deviceState.value === DeviceState.NOT_OPERATIONAL) {
      startDevice()
    } else if (deviceState.value === DeviceState.OPERATIONAL) {
      stopDevice()
    }
  }

  const toggleSession = () => {
    if (sessionState.value === SessionState.STOPPED) {
      startSession()
    } else if (sessionState.value === SessionState.STARTED) {
      stopSession()
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
        onClick={toggleSession}
        disabled={
          sessionState.value === SessionState.STARTING || sessionState.value === SessionState.STOPPING
        }
      >
        {sessionText()} session
      </StyledButton>
    </div>
  )
}

const StyledButton = styled.button`
  width: 200px;
  height: 50px;

  font-size: 1.00rem;
  padding: 0.8rem 0.5rem;
  margin-bottom: 0.8rem;
  border: none;
  border-radius: 5px;
  background-color: #007BFF;
  color: white;
  cursor: pointer;

  &:hover {
    background-color: #0056b3;
  }
  &:disabled {
    background-color: #CCCCCC;
    color: #888888;
  }
  &:hover:disabled {
    background-color: #CCCCCC;
  }
`
