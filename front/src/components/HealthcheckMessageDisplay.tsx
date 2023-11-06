import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { StyledPanel } from 'styles/General'

const HealthcheckMessagePanel = styled(StyledPanel)`
  width: 400px;
  height: 40px;
  position: fixed;
  top: 124px;
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

  let displayMessage

  /* Prioritize mTMS device healthcheck message over EEG healthcheck message IF
     the device is not disabled. If it is, do not show any mTMS device related messages.*/
  if (
    mtmsDeviceHealthcheck?.status.value !== HealthcheckStatus.READY &&
    mtmsDeviceHealthcheck?.status.value !== HealthcheckStatus.DISABLED
  ) {
    displayMessage = mtmsDeviceHealthcheck?.status_message
  } else {
    displayMessage = eegHealthcheck?.status_message
  }

  return (
    <HealthcheckMessagePanel>
      <Header>Status</Header>
      {displayMessage && <Message>{displayMessage}</Message>}
    </HealthcheckMessagePanel>
  )
}
