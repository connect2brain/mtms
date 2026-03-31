import React, { useContext } from 'react'
import styled from 'styled-components'

import { HealthContext, HealthStatus } from 'providers/HealthProvider'
import { StyledPanel } from 'styles/General'

interface StatusSquareProps {
  status: number
}

const HealthPanel = styled(StyledPanel)`
  width: 300px;
  height: 0px;
  position: fixed;
  top: 10px;
  right: 5px;
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-gap: 10px;
  z-index: 1000;
`

const StatusSquare = styled.div<StatusSquareProps>`
  width: 16px;
  height: 16px;
  display: inline-block;
  vertical-align: middle;
  background-color: ${({ status }) => {
    switch (status) {
      case HealthStatus.READY:
        return 'green'
      case HealthStatus.DEGRADED:
        return 'yellow'
      case HealthStatus.ERROR:
        return 'red'
      default:
        return 'grey'
    }
  }};
  border: 3px solid black;
  margin-right: 10px;
`

const StatusLine = styled.div`
  font-size: 0.9rem;
  font-weight: bold;
  margin-bottom: 8px;
`

export const HealthStatusDisplay: React.FC = () => {
  const { mtmsDeviceHealth } = useContext(HealthContext)

  return (
    <HealthPanel>
      <StatusLine>
        <StatusSquare
          status={
            mtmsDeviceHealth?.health_level !== undefined && mtmsDeviceHealth?.health_level !== null
              ? mtmsDeviceHealth?.health_level
              : -1
          }
        />
        mTMS device
      </StatusLine>
    </HealthPanel>
  )
}
