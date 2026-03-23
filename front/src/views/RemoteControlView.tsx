import React, { useRef, useState } from 'react'
import styled from 'styled-components'

import { RemoteControl } from 'components/remote_control/RemoteControl'
import { StimulationReadinessDisplay } from 'components/remote_control/StimulationReadinessDisplay'
import { PulseTable, PulseTableHandle } from 'components/remote_control/PulseTable'

export const RemoteControlView = () => {
  const pulseTableRef = useRef<PulseTableHandle>(null)
  const [rowCount, setRowCount] = useState(0)

  return (
    <RemoteControlPage>
      <ControlRow>
        <RemoteControl
          getTargetLists={() => pulseTableRef.current?.getTargetLists() ?? null}
          canStart={rowCount > 0}
        />
        <StimulationReadinessDisplay />
      </ControlRow>
      <PulseTableWrapper>
        <PulseTable ref={pulseTableRef} onRowCountChange={setRowCount} />
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

const ControlRow = styled.div`
  display: flex;
  flex-direction: row;
  align-items: flex-start;
  gap: 16px;
`

const PulseTableWrapper = styled.div`
  width: 100%;
`
