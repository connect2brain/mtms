import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import { systemStateSubscriber } from 'services/experiment'
import { ChannelState as ChannelStateType, SystemStateMessage } from 'types/fpga'
import { objectKeysToCamelCase } from 'utils'
import { ChannelState } from './ChannelState'

export const SystemState = () => {
  const [deviceState, setDeviceState] = useState<string>()
  const [experimentState, setExperimentState] = useState<string>()
  const [startupSequenceError, setStartupSequenceError] = useState<string>()

  const [channelStates, setChannelStates] = useState<ChannelStateType[]>([])

  useEffect(() => {
    systemStateSubscriber.subscribe(systemStateCallback)
  }, [])

  const systemStateCallback = (message: SystemStateMessage) => {
    const formattedMessage: SystemStateMessage = objectKeysToCamelCase(message)

    setDeviceState(formattedMessage.deviceState.toString())
    setExperimentState(formattedMessage.experimentState.toString())
    setStartupSequenceError(formattedMessage.startupSequenceError.toString())
    setChannelStates(formattedMessage.channelStates)
  }

  const channelStatesTable = () => {
    return (
      <ChannelTable>
        <Thead>
          <tr>
            <Th>Index</Th>
            <Th>Voltage</Th>
            <Th>Temperature</Th>
            <Th>Pulse count</Th>
            <Th>Error</Th>
          </tr>
        </Thead>
        <tbody>
          {channelStates
            .sort((a, b) => a.channelIndex - b.channelIndex)
            .map((channel) => (
              <ChannelState key={`channel-${channel.channelIndex}`} {...channel} />
            ))}
        </tbody>
      </ChannelTable>
    )
  }

  return (
    <div>
      <p>Device state: {deviceState}</p>
      <p>Experiment state: {experimentState}</p>
      <p>Startup Sequence Error: {startupSequenceError}</p>
      <ChannelTableContainer>{channelStatesTable()}</ChannelTableContainer>
    </div>
  )
}

const ChannelTableContainer = styled.div`
  overflow-y: auto;
  overflow-x: hidden;
  width: fit-content;
  max-height: 600px;
`

const ChannelTable = styled.table`
  border-collapse: collapse;
  table-layout: fixed;
  width: 100%;
`
const Thead = styled.thead`
  top: 0;
  margin: 0 0 0 0;
  width: 100%;
  z-index: 1;
`
const Th = styled.th`
  padding: 0.25rem 0.5rem;
  text-align: left;
  border-top: none !important;
  border-bottom: none !important;

  :last-of-type {
    box-shadow: inset 0 1px 0 ${(p) => p.theme.colors.gray}, inset 0 -1px 0 ${(p) => p.theme.colors.gray};
  }

  :not(:last-of-type) {
    box-shadow: inset 0 1px 0 ${(p) => p.theme.colors.gray}, inset 0 -1px 0 ${(p) => p.theme.colors.gray},
      inset -1px 0 0 ${(p) => p.theme.colors.gray};
  }
`
