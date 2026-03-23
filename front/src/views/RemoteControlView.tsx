import React, { useRef } from 'react'
import styled from 'styled-components'

import { RemoteControl } from 'components/remote_control/RemoteControl'
import { PulseTable, PulseTableHandle } from 'components/remote_control/PulseTable'

export const RemoteControlView = () => {
  const pulseTableRef = useRef<PulseTableHandle>(null)

  return (
    <RemoteControlPage>
      <RemoteControl getTargetLists={() => pulseTableRef.current?.getTargetLists() ?? null} />
      <PulseTableWrapper>
        <PulseTable ref={pulseTableRef} />
      </PulseTableWrapper>
    </RemoteControlPage>
  )
}

const RemoteControlPage = styled.div`
  width: 820px;
  min-height: 550px;
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 16px;
`

const PulseTableWrapper = styled.div`
  width: 100%;
`

