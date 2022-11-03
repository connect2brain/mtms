import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import { systemStateSubscriber } from 'services/experiment'
import {ChannelState as ChannelStateType, DeviceState, ExperimentState, SystemStateMessage} from 'types/fpga'
import { getKeyByValue, getTrueKeys, objectKeysToCamelCase } from 'utils'
import { ChannelState } from './ChannelState'

const initialState = {
  channel_states: [],
  system_error_cumulative: {
    heartbeat_error: false,
    latched_fault_error: false,
    powersupply_error: false,
    safety_bus_error: false,
    coil_error: false,
    emergency_button_error: false,
    door_error: false,
    charger_overvoltage_error: false,
    charger_overtemperature_error: false,
    monitored_voltage_over_maximum_error: false,
    patient_safety_error: false,
    device_safety_error: false,
    charger_powerup_error: false,
    opto_error: false,
    charger_power_enabled_twice_error: false,
  },
  system_error_current: {
    heartbeat_error: false,
    latched_fault_error: false,
    powersupply_error: false,
    safety_bus_error: false,
    coil_error: false,
    emergency_button_error: false,
    door_error: false,
    charger_overvoltage_error: false,
    charger_overtemperature_error: false,
    monitored_voltage_over_maximum_error: false,
    patient_safety_error: false,
    device_safety_error: false,
    charger_powerup_error: false,
    opto_error: false,
    charger_power_enabled_twice_error: false,
  },
  system_error_emergency: {
    heartbeat_error: false,
    latched_fault_error: false,
    powersupply_error: false,
    safety_bus_error: false,
    coil_error: false,
    emergency_button_error: false,
    door_error: false,
    charger_overvoltage_error: false,
    charger_overtemperature_error: false,
    monitored_voltage_over_maximum_error: false,
    patient_safety_error: false,
    device_safety_error: false,
    charger_powerup_error: false,
    opto_error: false,
    charger_power_enabled_twice_error: false,
  },
  startup_error: {
    error: 0,
  },
  device_state: {
    state: 0,
  },
  experiment_state: {
    state: 0,
  },
  time: 0,
}

export const SystemState = () => {
  const [systemState, setSystemState] = useState<SystemStateMessage>(initialState)

  useEffect(() => {
    console.log('Subscribed')
    systemStateSubscriber.subscribe(systemStateCallback)
  }, [])


  const systemStateCallback = (message: SystemStateMessage) => {
    console.log('new message', message)
    setSystemState(message)
  }

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
            .sort((a, b) => a.channel_index - b.channel_index)
            .map((channel) => (
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
      <p>Latest update: {new Date(systemState.time * 1000).toISOString()}</p>
      <p>Cumulative system errors: {getListValue(systemState.system_error_cumulative)}</p>
      <p>Current system errors: {getListValue(systemState.system_error_current)}</p>
      <p>Emergency system errors: {getListValue(systemState.system_error_emergency)}</p>
      <p>Startup error: {getKeyByValue(DeviceState, systemState.experiment_state) || 'No error'}</p>

      <p>Device state: {getKeyByValue(DeviceState, systemState.device_state.state) || 'No error'}</p>
      <p>Experiment state: {getKeyByValue(ExperimentState, systemState.experiment_state.state) || 'No error'}</p>

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
