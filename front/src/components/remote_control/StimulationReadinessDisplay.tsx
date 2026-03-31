import React, { useContext } from 'react'
import styled from 'styled-components'

import { TrialReadinessContext } from 'providers/TrialProvider'
import { SystemContext, DeviceState } from 'providers/SystemProvider'
import { StyledPanel } from 'styles/General'

const StatusPanel = styled(StyledPanel)`
  width: 125px;
  padding: 14px 20px;
  margin-top: 134px;
`

const StatusRow = styled.div`
  display: flex;
  align-items: center;
  gap: 10px;
`

const StatusSquare = styled.div<{ $status: boolean }>`
  width: 16px;
  height: 16px;
  border-radius: 50%;
  background-color: ${({ $status }) => {
    return $status ? 'green' : 'red'
  }};
  border: 3px solid black;
`

const StatusText = styled.div<{ $muted: boolean }>`
  color: ${({ $muted }) => ($muted ? '#959595' : '#333')};
  font-size: 0.95rem;
  font-weight: 600;
`

export const StimulationReadinessDisplay: React.FC = () => {
  const { trialReadiness } = useContext(TrialReadinessContext)
  const { systemState } = useContext(SystemContext)

  const charged = trialReadiness === true
  const statusText = charged ? 'Charged' : 'Not charged'

  const isDeviceOn =
    systemState?.device_state?.value !== undefined && systemState.device_state.value !== DeviceState.NOT_OPERATIONAL
  const muted = !isDeviceOn

  return (
    <StatusPanel>
      <StatusRow>
        <StatusSquare $status={charged} />
        <StatusText $muted={muted}>{statusText}</StatusText>
      </StatusRow>
    </StatusPanel>
  )
}
