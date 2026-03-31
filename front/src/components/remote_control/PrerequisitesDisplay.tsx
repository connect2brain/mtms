import React, { useContext } from 'react'
import styled from 'styled-components'

import { StyledPanel } from 'styles/General'
import { EegDeviceInfoContext } from 'providers/EegDeviceInfoProvider'
import { SystemContext, SessionState } from 'providers/SystemProvider'
import { RemoteControllerContext, RemoteControllerState } from 'providers/RemoteControllerProvider'

interface PrerequisitesDisplayProps {
  rowsDefined: boolean
}

export const PrerequisitesDisplay: React.FC<PrerequisitesDisplayProps> = ({ rowsDefined }) => {
  const { eegDeviceInfo } = useContext(EegDeviceInfoContext)
  const { session } = useContext(SystemContext)
  const { state: remoteControllerState } = useContext(RemoteControllerContext)

  const isRemoteControlActive =
    remoteControllerState !== null && remoteControllerState !== RemoteControllerState.NOT_STARTED

  const isStreaming = Boolean(eegDeviceInfo?.is_streaming)
  const isSessionAvailable = session === null || session.state === SessionState.STOPPED

  const conditions = [
    { label: 'EEG streaming', met: isStreaming },
    { label: 'At least one pulse defined', met: rowsDefined },
    { label: 'Session available', met: isSessionAvailable },
  ]

  return (
    <PrerequisitesPanel>
      <Title>Prerequisites</Title>
      {conditions.map(({ label, met }) => (
        <ConditionRow key={label}>
          <ConditionMark $met={met || isRemoteControlActive}>{met || isRemoteControlActive ? '✓' : '✗'}</ConditionMark>
          <ConditionLabel>{label}</ConditionLabel>
        </ConditionRow>
      ))}
    </PrerequisitesPanel>
  )
}

const PrerequisitesPanel = styled(StyledPanel)`
  height: 154px;
  padding: 15px 20px;
  display: flex;
  flex-direction: column;
  justify-content: center;
  gap: 10px;
`

const Title = styled.div`
  font-size: 1.2rem;
  font-weight: 700;
  color: #333;
  margin-bottom: 10px;
`

const ConditionRow = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
`

const ConditionMark = styled.span<{ $met: boolean }>`
  color: ${({ $met }) => ($met ? 'green' : 'red')};
  font-weight: 700;
  font-size: 1rem;
  width: 14px;
  line-height: 1;
  display: inline-block;
  text-align: center;
`

const ConditionLabel = styled.span`
  font-size: 1.0rem;
  color: #333;
`
