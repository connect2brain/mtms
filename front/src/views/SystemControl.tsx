import React, { useEffect, useState } from 'react'
import {
  chargeFeedbackSubscriber,
  dischargeFeedbackSubscriber,
  pulseFeedbackSubscriber,
  triggerOutFeedbackSubscriber,
  systemStateSubscriber,
} from 'services/experiment'
import { SystemStateMessage } from 'types/mtmsDevice'
import {
  PulseFeedbackMessage,
  Feedback,
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  TriggerOutFeedbackMessage,
} from 'types/event'
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
    triggerOutFeedbackSubscriber.subscribe(triggerOutFeedbackCallback)
  }, [])

  const pulseFeedbackCallback = (message: PulseFeedbackMessage) => {
    setFeedback({
      ...message,
      type: 'pulse',
    })
  }

  const chargeFeedbackCallback = (message: ChargeFeedbackMessage) => {
    setFeedback({
      ...message,
      type: 'charge',
    })
  }

  const dischargeFeedbackCallback = (message: DischargeFeedbackMessage) => {
    setFeedback({
      ...message,
      type: 'discharge',
    })
  }

  const triggerOutFeedbackCallback = (message: TriggerOutFeedbackMessage) => {
    setFeedback({
      ...message,
      type: 'triggerOut',
    })
  }

  const systemStateCallback = (message: SystemStateMessage) => {
    setSystemState(message)
  }

  return (
    <div>
      <ExperimentControl deviceState={systemState.device_state} experimentState={systemState.experiment_state} />
      <SystemState systemState={systemState} />
      <hr />
      <EventFeedbacks feedback={feedback} />
    </div>
  )
}
