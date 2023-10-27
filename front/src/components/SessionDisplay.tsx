import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { StyledPanel } from 'styles/General'
import { getKeyByValue } from 'utils'

import { SessionControl } from 'components/SessionControl'

import {
  SystemContext,
  SessionState,
  HumanReadableSessionState,
} from 'providers/SystemProvider'

const SessionPanel = styled(StyledPanel)`
  width: 300px;
  height: 150px;
  position: fixed;
  top: 125px;
  right: 5px;
  z-index: 1000;
`

const StateRow = styled.div`
  display: flex;
  justify-content: space-between;
  margin-bottom: 0.5rem;
  margin-right: 15px;
`

const StateTitle = styled.span`
  font-weight: bold;
  color: #333;
  margin-right: 1rem;
`

const StateValue = styled.span``

export const SessionDisplay: React.FC = () => {
  const { systemState } = useContext(SystemContext)

  const [latestUpdate, setLatestUpdate] = useState<Date>()

  useEffect(() => {
    setLatestUpdate(new Date())
  }, [systemState])

  const formatDate = (isoString: any) => {
    const date = new Date(isoString)

    const year = date.getUTCFullYear()
    const month = String(date.getUTCMonth() + 1).padStart(2, '0')
    const day = String(date.getUTCDate()).padStart(2, '0')

    const hours = String(date.getUTCHours()).padStart(2, '0')
    const minutes = String(date.getUTCMinutes()).padStart(2, '0')
    const seconds = String(date.getUTCSeconds()).padStart(2, '0')

    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`
  }

  const getHumanReadableSessionState = (sessionState: any, value: any) => {
    const key = getKeyByValue(SessionState, value)
    if (key) {
      return HumanReadableSessionState[key as keyof typeof HumanReadableSessionState] || 'Unknown state'
    } else {
      return 'Unknown state'
    }
  }

  return (
    <SessionPanel>
      <StateRow>
        <StateTitle>Time</StateTitle>
        <StateValue>{latestUpdate ? formatDate(latestUpdate.toISOString()) : ''}</StateValue>
      </StateRow>
      <br />
      <StateRow>
        <StateTitle>Session</StateTitle>
        <StateValue>{getHumanReadableSessionState(SessionState, systemState?.session_state.value)}</StateValue>
      </StateRow>
      <StateRow>
        <StateTitle>Session time</StateTitle>
        <StateValue>{systemState?.time.toFixed(1)} s</StateValue>
      </StateRow>
      <br />
      <SessionControl />
    </SessionPanel>
  )
}
