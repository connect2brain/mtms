import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import { systemStateSubscriber } from 'services/experiment'
import {
  ChannelState as ChannelStateType,
  DeviceState,
  ExperimentState,
  SystemStateMessage,
  StartupError,
} from 'types/fpga'
import { getKeyByValue, getTrueKeys, objectKeysToCamelCase } from 'utils'
import { SystemState } from 'components/SystemState'
import { ExperimentControl } from 'components/ExperimentControl'

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
    value: 0,
  },
  device_state: {
    value: 0,
  },
  experiment_state: {
    value: 0,
  },
  time: 0,
}

export const SystemControl = () => {
  const [systemState, setSystemState] =
    useState<SystemStateMessage>(initialState)

  //const [deviceState, setDeviceState] = useState<number>(0)
  //const [experimentState, setExperimentState] = useState<number>(0)

  useEffect(() => {
    console.log('Subscribed')
    systemStateSubscriber.subscribe(systemStateCallback)
  }, [])

  const systemStateCallback = (message: SystemStateMessage) => {
    //console.log('new message', message)
    setSystemState(message)
  }

  return (
    <div>
      <ExperimentControl
        deviceState={systemState.device_state.value}
        experimentState={systemState.experiment_state.value}
      />
      <SystemState systemState={systemState} />
    </div>
  )
}
