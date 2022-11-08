import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import {
  chargeFeedbackSubscriber,
  dischargeFeedbackSubscriber,
  pulseFeedbackSubscriber,
  signalOutFeedbackSubscriber,
  systemStateSubscriber,
} from 'services/experiment'
import {
  ChannelState as ChannelStateType,
  DeviceState,
  ExperimentState,
  SystemStateMessage,
  StartupError,
  PulseFeedbackMessage,
  Feedback,
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  SignalOutFeedbackMessage,
} from 'types/fpga'
import { getKeyByValue, getTrueKeys, objectKeysToCamelCase } from 'utils'
import { SystemState } from 'components/SystemState'
import { ExperimentControl } from 'components/ExperimentControl'
import { EventFeedbacks } from 'components/EventFeedbacks'

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
  const [systemState, setSystemState] = useState<SystemStateMessage>(initialState)
  const [feedback, setFeedback] = useState<Feedback>()

  useEffect(() => {
    console.log('Subscribed')
    systemStateSubscriber.subscribe(systemStateCallback)
    pulseFeedbackSubscriber.subscribe(pulseFeedbackCallback)
    chargeFeedbackSubscriber.subscribe(chargeFeedbackCallback)
    dischargeFeedbackSubscriber.subscribe(dischargeFeedbackCallback)
    signalOutFeedbackSubscriber.subscribe(signalOutFeedbackCallback)
  }, [])

  const pulseFeedbackCallback = (message: PulseFeedbackMessage) => {
    console.log('new message', message)
    setFeedback({
      ...message,
      type: 'pulse',
    })
  }

  const chargeFeedbackCallback = (message: ChargeFeedbackMessage) => {
    console.log('new message', message)
    setFeedback({
      ...message,
      type: 'charge',
    })
  }

  const dischargeFeedbackCallback = (message: DischargeFeedbackMessage) => {
    console.log('new message', message)
    setFeedback({
      ...message,
      type: 'discharge',
    })
  }

  const signalOutFeedbackCallback = (message: SignalOutFeedbackMessage) => {
    console.log('new message', message)
    setFeedback({
      ...message,
      type: 'signalOut',
    })
  }

  const systemStateCallback = (message: SystemStateMessage) => {
    setSystemState(message)
  }

  return (
    <div>
      <ExperimentControl
        deviceState={systemState.device_state.value}
        experimentState={systemState.experiment_state.value}
      />
      <SystemState systemState={systemState} />
      <EventFeedbacks feedback={feedback} />
    </div>
  )
}
