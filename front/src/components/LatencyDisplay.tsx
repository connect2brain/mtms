import React, { useContext, useEffect, useState } from 'react'
import styled from 'styled-components'

import { StyledPanel, StateRow, StateTitle, StateValue } from 'styles/General'

import { PipelineContext } from 'providers/PipelineProvider'
import { SystemContext, SessionState } from 'providers/SystemProvider'

const LatencyPanel = styled(StyledPanel)`
  width: 300px;
  height: 29px;
  position: fixed;
  top: 975px;
  right: 5px;
  z-index: 1000;
`

export const LatencyDisplay: React.FC = () => {
  const { latency, setLatency } = useContext(PipelineContext)
  const { session } = useContext(SystemContext)

  const sessionState = session?.state

  const formattedLatency = (latency && (latency.latency * 1000).toFixed(1) + ' ms') || '\u2013'
  const formattedSampleTime = (latency && latency.sample_time.toFixed(1) + ' s') || '\u2013'

  /* XXX: Not quite the right place to reset latency here. */
  useEffect(() => {
    if (sessionState?.value === SessionState.STOPPED) {
      setLatency(null)
    }
  }, [sessionState])

  return (
    <LatencyPanel>
      <StateRow>
        <StateTitle>Decision time:</StateTitle>
        <StateValue>{sessionState?.value === SessionState.STARTED ? formattedSampleTime : '\u2013'}</StateValue>
      </StateRow>
      <StateRow>
        <StateTitle>Round-trip latency:</StateTitle>
        <StateValue>{sessionState?.value === SessionState.STARTED ? formattedLatency : '\u2013'}</StateValue>
      </StateRow>
    </LatencyPanel>
  )
}
