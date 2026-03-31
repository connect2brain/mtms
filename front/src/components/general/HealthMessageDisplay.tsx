import React, { useContext } from 'react'
import styled from 'styled-components'

import { HealthContext, HealthStatus } from 'providers/HealthProvider'
import { StyledPanel } from 'styles/General'

const HealthMessagePanel = styled(StyledPanel)`
  width: 400px;
  height: 45px;
  position: fixed;
  top: 80px;
  right: 5px;
  z-index: 1000;
`

const Header = styled.div`
  color: #333;
  font-weight: bold;
  font-size: 1.1rem;
  margin-bottom: 0.5rem;
`

const Message = styled.div`
  color: #333;
  font-size: 0.9rem;
  padding: 5px;
  border-bottom: 0px;
  transition: opacity 0.3s;
`

const getHealthDisplay = (health: { health_level: number; message: string } | null): { prefix: string | null; message: string } | null => {
  if (health === null || health.health_level === HealthStatus.READY) return null
  if (health.health_level === HealthStatus.ERROR) return { prefix: 'Error:', message: health.message }
  return { prefix: '', message: health.message }
}

export const HealthMessageDisplay: React.FC = () => {
  const { eegHealth, mtmsDeviceHealth, remoteControllerHealth, timebaseCalibratorHealth } = useContext(HealthContext)

  // Prioritize mtmsDeviceHealth > eegHealth > remoteControllerHealth > timebaseCalibratorHealth
  const healthDisplay =
    getHealthDisplay(mtmsDeviceHealth) ??
    getHealthDisplay(eegHealth) ??
    getHealthDisplay(remoteControllerHealth) ??
    getHealthDisplay(timebaseCalibratorHealth)

  return (
    <HealthMessagePanel>
      <Header>Status</Header>
      <Message>
        {healthDisplay ? (
          <><b>{healthDisplay.prefix}</b> {healthDisplay.message}</>
        ) : (
          <b>Ready</b>
        )}
      </Message>
    </HealthMessagePanel>
  )
}
