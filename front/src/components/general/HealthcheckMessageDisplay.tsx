import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { NeuroSimoContext } from 'providers/NeuroSimoProvider'
import { StyledPanel } from 'styles/General'

const HealthcheckMessagePanel = styled(StyledPanel)`
  width: 400px;
  height: 45px;
  position: fixed;
  top: 85px;
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
  const { eegHealthcheck, mtmsDeviceHealthcheck } = useContext(HealthcheckContext)
  const { deciderEnabled } = useContext(NeuroSimoContext)

  let displayMessage

  // Prioritize mtmsDeviceHealthcheck > eegHealthcheck
  if (
    mtmsDeviceHealthcheck?.status.value !== HealthcheckStatus.READY &&
    mtmsDeviceHealthcheck?.status.value !== HealthcheckStatus.DISABLED
  ) {
    displayMessage = mtmsDeviceHealthcheck?.actionable_message
  } else if (eegHealthcheck?.status.value !== HealthcheckStatus.READY) {
    displayMessage = eegHealthcheck?.actionable_message
  } else if (deciderEnabled) {
    displayMessage = 'Please disable Decider to run experiments'
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
