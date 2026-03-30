import React, { useContext } from 'react'
import styled from 'styled-components'

import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { StyledPanel } from 'styles/General'

const HealthcheckMessagePanel = styled(StyledPanel)`
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

export const HealthcheckMessageDisplay: React.FC = () => {
  const { eegHealthcheck, mtmsDeviceHealthcheck, remoteControllerHealthcheck } = useContext(HealthcheckContext)

  let displayMessage

  // Prioritize mtmsDeviceHealthcheck > eegHealthcheck > remoteControllerHealthcheck
  if (
    mtmsDeviceHealthcheck?.status !== HealthcheckStatus.READY &&
    mtmsDeviceHealthcheck?.status !== HealthcheckStatus.DISABLED
  ) {
    displayMessage = mtmsDeviceHealthcheck?.actionable_message
  } else if (eegHealthcheck?.status !== HealthcheckStatus.READY) {
    displayMessage = eegHealthcheck?.actionable_message
  } else if (remoteControllerHealthcheck?.status === HealthcheckStatus.NOT_READY || remoteControllerHealthcheck?.status === HealthcheckStatus.ERROR) {
    displayMessage = remoteControllerHealthcheck?.actionable_message
  } else {
    displayMessage = 'Ready'
  }

  return (
    <HealthcheckMessagePanel>
      <Header>Status</Header>
      {displayMessage && <Message>{displayMessage}</Message>}
    </HealthcheckMessagePanel>
  )
}
