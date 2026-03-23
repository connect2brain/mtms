import React, { useContext } from 'react'
import styled from 'styled-components'

import { TrialReadinessContext } from 'providers/TrialProvider'
import { StyledPanel } from 'styles/General'

const StatusPanel = styled(StyledPanel)`
  width: 250px;
  position: fixed;
  top: 390px;
  right: 5px;
  z-index: 1000;
  padding: 15px 20px;
`

const Header = styled.div`
  color: #333;
  font-weight: bold;
  font-size: 1.05rem;
  margin-bottom: 0.65rem;
`

const StatusRow = styled.div`
  display: flex;
  align-items: center;
  gap: 10px;
`

const StatusSquare = styled.div<{ $status: 'ready' | 'not_ready' | 'unknown' }>`
  width: 16px;
  height: 16px;
  border-radius: 50%;
  background-color: ${({ $status }) => {
    switch ($status) {
      case 'ready':
        return 'green'
      case 'not_ready':
        return 'red'
      default:
        return 'grey'
    }
  }};
  border: 3px solid black;
`

const StatusText = styled.div`
  color: #333;
  font-size: 0.95rem;
  font-weight: 600;
`

export const MTMSStatusDisplay: React.FC = () => {
  const { trialReadiness } = useContext(TrialReadinessContext)

  const status: 'ready' | 'not_ready' | 'unknown' =
    trialReadiness === null ? 'unknown' : trialReadiness ? 'ready' : 'not_ready'

  const statusText = status === 'ready' ? 'Trial ready' : status === 'not_ready' ? 'Trial not ready' : 'Trial unknown'

  return (
    <StatusPanel>
      <Header>mTMS status</Header>
      <StatusRow>
        <StatusSquare $status={status} />
        <StatusText>{statusText}</StatusText>
      </StatusRow>
    </StatusPanel>
  )
}

