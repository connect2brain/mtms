import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import {
  chargeFeedbackSubscriber,
  dischargeFeedbackSubscriber,
  pulseFeedbackSubscriber,
  triggerOutFeedbackSubscriber,
} from 'ros/feedback'

import {
  PulseFeedbackMessage,
  Feedback,
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  TriggerOutFeedbackMessage,
} from 'types/event'

import { SystemState } from 'components/system/SystemState'
import { DeviceControl } from 'components/system/DeviceControl'
import { EventFeedbacks } from 'components/system/EventFeedbacks'

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
  session_state: {
    value: 0,
  },
  time: 0,
}

export const SystemView = () => {
  const [feedback, setFeedback] = useState<Feedback>()

  useEffect(() => {
    console.log('Subscribed')
    pulseFeedbackSubscriber.subscribe(pulseFeedbackCallback)
    chargeFeedbackSubscriber.subscribe(chargeFeedbackCallback)
    dischargeFeedbackSubscriber.subscribe(dischargeFeedbackCallback)
    triggerOutFeedbackSubscriber.subscribe(triggerOutFeedbackCallback)

    return () => {
      pulseFeedbackSubscriber.unsubscribe(pulseFeedbackCallback)
      chargeFeedbackSubscriber.unsubscribe(chargeFeedbackCallback)
      dischargeFeedbackSubscriber.unsubscribe(dischargeFeedbackCallback)
      triggerOutFeedbackSubscriber.unsubscribe(triggerOutFeedbackCallback)
    }
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

  return (
    <>
      <Wrapper>
        <PanelA>
          <DeviceControl />
        </PanelA>
        <VerticalDividedPanelB>
          <SystemState />
        </VerticalDividedPanelB>
        <PanelC>
          <EventFeedbacks feedback={feedback} />
        </PanelC>
      </Wrapper>
    </>
  )
}

const Wrapper = styled.div`
  display: grid;
  grid-template-columns: minmax(20rem, 26rem) minmax(24rem, 38rem);
  grid-template-rows: auto 1fr;
  gap: 1rem;
  padding: 1rem;
`

const styledPanel = `
  padding: 1rem;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

const PanelA = styled.div`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  ${styledPanel}
`

const VerticalDividedPanelB = styled.div`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  ${styledPanel}
`

const PanelC = styled.div`
  grid-row: 2 / 3;
  grid-column: 1 / 3;
  ${styledPanel}
`
