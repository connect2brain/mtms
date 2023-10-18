import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { ChannelState as ChannelStateType, DeviceState, HumanReadableDeviceState, SessionState, HumanReadableSessionState, SystemStateMessage } from 'types/mtmsDevice'
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
        <colgroup>
          <ColStyle className="indexCol" />
          <ColStyle className="voltageCol" />
          <ColStyle className="pulseCol" />
          <ColStyle className="errorCol" />
        </colgroup>
        <Thead>
          <tr>
            <Th>#</Th>
            <Th>Voltage (V)</Th>
            {/*<Th>Temperature</Th>*/}
            <Th># of pulses</Th>
            <Th>Errors</Th>
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
        return <span key={key}>{key}, </span>
      })
    } else {
      /* No errors, do not display anything. */
      return <span></span>
    }
  }

  const formatDate = (isoString: any) => {
    const date = new Date(isoString)

    const year = date.getUTCFullYear()
    const month = String(date.getUTCMonth() + 1).padStart(2, '0')
    const day = String(date.getUTCDate()).padStart(2, '0')

    const hours = String(date.getUTCHours()).padStart(2, '0')
    const minutes = String(date.getUTCMinutes()).padStart(2, '0')
    const seconds = String(date.getUTCSeconds()).padStart(2, '0')

    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`
  }

  const getHumanReadableDeviceState = (deviceState: any, value: any) => {
    const key = getKeyByValue(DeviceState, value)
    if (key) {
      return HumanReadableDeviceState[key as keyof typeof HumanReadableDeviceState] || 'Unknown state'
    } else {
      return 'Unknown state'
    }
  }

  const getHumanReadableSessionState = (sessionState: any, value: any) => {
    const key = getKeyByValue(SessionState, value)
    if (key) {
      return HumanReadableSessionState[key as keyof typeof HumanReadableSessionState] || 'Unknown state'
    } else {
      return 'Unknown state'
    }
  }

  return (
    <div>
      <StateRow>
        <StateTitle>Device</StateTitle>
        <StateValue>{getHumanReadableDeviceState(DeviceState, systemState.device_state.value)}</StateValue>
      </StateRow>
      <StateRow>
        <StateTitle>Session</StateTitle>
        <StateValue>{getHumanReadableSessionState(SessionState, systemState.session_state.value)}</StateValue>
      </StateRow>
      <br />
      <StateRow>
        <StateTitle>Time</StateTitle>
        <StateValue>{latestUpdate ? formatDate(latestUpdate.toISOString()) : ''}</StateValue>
      </StateRow>
      <StateRow>
        <StateTitle>Session time</StateTitle>
        <StateValue>{systemState.time.toFixed(1)} s</StateValue>
      </StateRow>
      <br />
      <ErrorTitle>Errors</ErrorTitle>
      <ErrorsContainer>
        <ErrorItem>Current: {getListValue(systemState.system_error_current)}</ErrorItem>
        <ErrorItem>Cumulative: {getListValue(systemState.system_error_cumulative)}</ErrorItem>
        <ErrorItem>Emergency: {getListValue(systemState.system_error_emergency)}</ErrorItem>
        <ErrorItem>
          Startup: {' '}
          {getKeyByValueExcluding(systemState.startup_error, 'value', systemState.startup_error.value) || ''}
        </ErrorItem>
      </ErrorsContainer>
      <ChannelTitle>Channels</ChannelTitle>
      <ChannelTableContainer>{channelStatesTable()}</ChannelTableContainer>
    </div>
  )
}

const StateRow = styled.div`
  display: flex;
  justify-content: space-between;
  margin-bottom: 0.5rem;
`

const StateTitle = styled.span`
  font-weight: bold;
  color: #333;
  margin-right: 1rem;
`

const StateValue = styled.span``

const ErrorsContainer = styled.div`
  margin-top: 1rem;
  margin-left: 1rem;
`

const ErrorTitle = styled.span`
  font-weight: bold;
  color: #333;
  margin-right: 1rem;
  margin-bottom: 0.5rem;
`

const ErrorItem = styled.p`
  font-size: 0.9rem;
  font-weight: bold;
  color: #444;
  margin-left: 0.5rem;
`

const ChannelTitle = styled.h3`
  font-weight: bold;
  color: #333;
`

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

const ColStyle = styled.col`
  &.indexCol {
    width: 10%;
  }
  &.voltageCol {
    width: 30%;
  }
  &.pulseCol {
    width: 30%;
  }
  &.errorCol {
    width: 30%;
  }
`

const Thead = styled.thead`
  top: 0;
  margin: 0 0 0 0;
  width: 100%;
  z-index: 1;
  background-color: #fff;
`

const Th = styled.th`
  padding: 0.25rem 0.5rem;
  text-align: right;
  border-top: none !important;
  border-bottom: none !important;
`
