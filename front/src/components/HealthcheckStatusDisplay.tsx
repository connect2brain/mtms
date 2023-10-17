import React, { useContext } from 'react'
import styled from 'styled-components'
import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'

interface StatusSquareProps {
  status: number
}

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
    <div>
      <StatusLine>
        <StatusSquare status={mtmsDeviceHealthcheck?.status.value || -1} />
        mTMS device
      </StatusLine>
      <StatusLine>
        <StatusSquare status={eegHealthcheck?.status.value || -1} />
        EEG
      </StatusLine>
    </div>
  )
}
