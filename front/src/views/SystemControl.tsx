import React, { useEffect, useState } from 'react'
import {
  chargeFeedbackSubscriber,
  dischargeFeedbackSubscriber,
  pulseFeedbackSubscriber,
  signalOutFeedbackSubscriber,
  systemStateSubscriber,
} from 'services/experiment'
import {
  SystemStateMessage,
  PulseFeedbackMessage,
  Feedback,
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  SignalOutFeedbackMessage,
} from 'types/fpga'
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
    NO_ERROR: 0,
    UART_INITIALIZATION_ERROR: 1,
    BOARD_STARTUP_ERROR: 2,
    BOARD_STATUS_MESSAGE_ERROR: 3,
    SAFETY_MONITOR_ERROR: 4,
    DISCHARGE_CONTROLLER_ERROR: 5,
    CHARGER_ERROR: 6,
    SENSORBOARD_ERROR: 7,
    DISCHARGE_CONTROLLER_VOLTAGE_ERROR: 8,
    CHARGER_VOLTAGE_ERROR: 9,
    IGBT_FEEDBACK_ERROR: 10,
    TEMPERATURE_SENSOR_PRESENCE_ERROR: 11,
    COIL_MEMORY_PRESENCE_ERROR: 12,
  },
  device_state: {
    value: 0,
    NOT_OPERATIONAL: 0,
    STARTUP: 1,
    OPERATIONAL: 2,
    SHUTDOWN: 3,
  },
  experiment_state: {
    value: 0,
    STOPPED: 0,
    STARTING: 1,
    STARTED: 2,
    STOPPING: 3,
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

  const signalOutFeedbackCallback = (message: SignalOutFeedbackMessage) => {
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
      <ExperimentControl deviceState={systemState.device_state} experimentState={systemState.experiment_state} />
      <SystemState systemState={systemState} />
      <hr />
      <EventFeedbacks feedback={feedback} />
    </div>
  )
}
