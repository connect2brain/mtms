import ROSLIB from '@foxglove/roslibjs'
import { ros } from './ros'

import {
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  PulseFeedbackMessage,
  TriggerOutFeedbackMessage,
} from '../types/event'

interface MTMSEventMessage extends ROSLIB.Message {
  when_to_execute: number
  processing_start_time: number
  event_type: number
}

export const eventSubscriber = new ROSLIB.Topic<MTMSEventMessage>({
  ros: ros,
  name: '/mtms/events',
  messageType: 'ui_interfaces/Event',
})

export const pulseFeedbackSubscriber = new ROSLIB.Topic<PulseFeedbackMessage>({
  ros: ros,
  name: '/event/pulse_feedback',
  messageType: 'event_interfaces/PulseFeedback',
})

export const chargeFeedbackSubscriber = new ROSLIB.Topic<ChargeFeedbackMessage>({
  ros: ros,
  name: '/event/charge_feedback',
  messageType: 'event_interfaces/ChargeFeedback',
})

export const dischargeFeedbackSubscriber = new ROSLIB.Topic<DischargeFeedbackMessage>({
  ros: ros,
  name: '/event/discharge_feedback',
  messageType: 'event_interfaces/DischargeFeedback',
})

export const triggerOutFeedbackSubscriber = new ROSLIB.Topic<TriggerOutFeedbackMessage>({
  ros: ros,
  name: '/event/trigger_out_feedback',
  messageType: 'event_interfaces/TriggerOutFeedback',
})
