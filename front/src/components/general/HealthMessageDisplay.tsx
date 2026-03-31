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

export const HealthMessageDisplay: React.FC = () => {
  const { eegHealth, mtmsDeviceHealth, remoteControllerHealth, timebaseCalibratorHealth } = useContext(HealthContext)

  let displayMessage = 'Ready'

  // Prioritize mtmsDeviceHealth > eegHealth > remoteControllerHealth > timebaseCalibratorHealth
  if (mtmsDeviceHealth !== null && mtmsDeviceHealth.health_level !== HealthStatus.READY) {
    displayMessage = mtmsDeviceHealth?.message
  } else if (eegHealth !== null && eegHealth.health_level !== HealthStatus.READY) {
    displayMessage = eegHealth?.message
  } else if (remoteControllerHealth !== null && remoteControllerHealth.health_level !== HealthStatus.READY) {
    displayMessage = remoteControllerHealth?.message
  } else if (timebaseCalibratorHealth !== null && timebaseCalibratorHealth.health_level !== HealthStatus.READY) {
    displayMessage = timebaseCalibratorHealth?.message
  }

  return (
    <HealthMessagePanel>
      <Header>Status</Header>
      {displayMessage && <Message>{displayMessage}</Message>}
    </HealthMessagePanel>
  )
}
