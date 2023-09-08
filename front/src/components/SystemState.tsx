import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import { ChannelState as ChannelStateType, DeviceState, SessionState, SystemStateMessage } from 'types/mtmsDevice'
import { getKeyByValue, getKeyByValueExcluding, getTrueKeys } from 'utils'
import { ChannelState } from './ChannelState'

type Props = {
  systemState: SystemStateMessage
}

export const SystemState = ({ systemState }: Props) => {
  const [latestUpdate, setLatestUpdate] = useState<Date>()

  useEffect(() => {
    setLatestUpdate(new Date())
  }, [systemState])

  const channelStatesTable = () => {
    return (
      <ChannelTable>
        <Thead>
          <tr>
            <Th>Index</Th>
            <Th>Voltage</Th>
            {/*<Th>Temperature</Th>*/}
            <Th>Pulse count</Th>
            <Th>Error</Th>
          </tr>
        </Thead>
        <tbody>
          {systemState.channel_states
            .sort((a: ChannelStateType, b: ChannelStateType) => a.channel_index - b.channel_index)
            .map((channel: ChannelStateType) => (
              <ChannelState key={`channel-${channel.channel_index}`} {...channel} />
            ))}
        </tbody>
      </ChannelTable>
    )
  }

  const getListValue = (object: any) => {
    const keys: string[] = getTrueKeys(object)

    if (keys.length > 0) {
      return keys.map((key) => {
        return <span key={key}>{key}</span>
      })
    } else {
      return <span>No error(s)</span>
    }
  }

  return (
    <div>
      <p>Device state: {getKeyByValue(DeviceState, systemState.device_state.value) || 'No error'}</p>
      <p>Session state: {getKeyByValue(SessionState, systemState.session_state.value) || 'No error'}</p>

      <br />

      <p>Latest update: {latestUpdate?.toISOString()}</p>
      <p>System time: {systemState.time} s</p>
      <p>Cumulative system errors: {getListValue(systemState.system_error_cumulative)}</p>
      <p>Current system errors: {getListValue(systemState.system_error_current)}</p>
      <p>Emergency system errors: {getListValue(systemState.system_error_emergency)}</p>
      <p>
        Startup error:{' '}
        {getKeyByValueExcluding(systemState.startup_error, 'value', systemState.startup_error.value) || 'No error'}
      </p>

      <h3>Channels</h3>
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
