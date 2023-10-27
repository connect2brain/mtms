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

  return (
    <HealthcheckMessagePanel>
      <Header>Status</Header>
      {eegHealthcheck?.status_message && <Message>{eegHealthcheck.status_message}</Message>}
      {mtmsDeviceHealthcheck?.status_message && <Message>{mtmsDeviceHealthcheck.status_message}</Message>}
    </HealthcheckMessagePanel>
  )
}
