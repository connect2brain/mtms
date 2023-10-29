import React, { useContext } from 'react'
import styled from 'styled-components'

import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { StyledPanel } from 'styles/General'

interface StatusSquareProps {
  status: number
}

const HealthcheckPanel = styled(StyledPanel)`
  width: 155px;
  height: 40px;
  position: fixed;
  top: 10px;
  right: 5px;
  z-index: 1000;
`

const StatusSquare = styled.div<StatusSquareProps>`
  width: 16px;
  height: 16px;
  display: inline-block;
  vertical-align: middle;
  background-color: ${({ status }) => {
    switch (status) {
      case HealthcheckStatus.READY:
        return 'green'
      case HealthcheckStatus.NOT_READY:
        return 'yellow'
      case HealthcheckStatus.DISABLED:
        return 'orange'
      case HealthcheckStatus.ERROR:
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

export const HealthcheckStatusDisplay: React.FC = () => {
  const { eegHealthcheck, mtmsDeviceHealthcheck } = useContext(HealthcheckContext)

  return (
    <HealthcheckPanel>
      <StatusLine>
        <StatusSquare
          status={
            mtmsDeviceHealthcheck?.status.value !== undefined && mtmsDeviceHealthcheck?.status.value !== null
              ? mtmsDeviceHealthcheck?.status.value
              : -1
          }
        />
        mTMS device
      </StatusLine>
      <StatusLine>
        <StatusSquare
          status={
            eegHealthcheck?.status.value !== undefined && eegHealthcheck?.status.value !== null
              ? eegHealthcheck?.status.value
              : -1
          }
        />
        EEG
      </StatusLine>
    </HealthcheckPanel>
  )
}
