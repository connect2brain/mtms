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

  let displayMessage

  // Prioritize mtmsDeviceHealth > eegHealth > remoteControllerHealth > timebaseCalibratorHealth
  //
  // XXX: This whole health logic would need some work.
  if (mtmsDeviceHealth?.health_level !== HealthStatus.READY) {
    displayMessage = mtmsDeviceHealth?.message
  } else if (eegHealth?.health_level !== HealthStatus.READY && eegHealth !== null) {
    displayMessage = eegHealth?.message
  } else if (remoteControllerHealth?.health_level !== HealthStatus.READY) {
    displayMessage = remoteControllerHealth?.message
  } else if (timebaseCalibratorHealth?.health_level !== HealthStatus.READY && timebaseCalibratorHealth !== null) {
    displayMessage = timebaseCalibratorHealth?.message
  } else {
    displayMessage = 'Ready'
  }

  return (
    <HealthMessagePanel>
      <Header>Status</Header>
      {displayMessage && <Message>{displayMessage}</Message>}
    </HealthMessagePanel>
  )
}
