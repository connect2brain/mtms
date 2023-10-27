import React from 'react'

import { startSession, stopSession } from 'ros/services/session'
import { DeviceState, DeviceStateMessage, SessionState, SessionStateMessage } from 'types/mtmsDevice'
import { StyledButton } from 'styles/General'

type Props = {
  deviceState: DeviceStateMessage
  sessionState: SessionStateMessage
}

export const SessionControl = ({ deviceState, sessionState }: Props) => {
  const sessionText = () => {
    if (sessionState.value === SessionState.STOPPED) return 'Start'
    if (sessionState.value === SessionState.STARTING) return 'Starting'
    if (sessionState.value === SessionState.STARTED) return 'Stop'
    if (sessionState.value === SessionState.STOPPING) return 'Stopping'
    return '???'
  }

  const toggleSession = () => {
    if (sessionState.value === SessionState.STOPPED) {
      startSession()
    } else if (sessionState.value === SessionState.STARTED) {
      stopSession()
    }
  }

  return (
    <>
      <StyledButton
        onClick={toggleSession}
        disabled={
          deviceState.value !== DeviceState.OPERATIONAL ||
          sessionState.value === SessionState.STARTING ||
          sessionState.value === SessionState.STOPPING
        }
      >
        {sessionText()} session
      </StyledButton>
    </>
)
}
