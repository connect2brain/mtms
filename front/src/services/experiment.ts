import ROSLIB from 'roslib'
import { ros } from './ros'
import {
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  PulseFeedbackMessage,
  TriggerOutFeedbackMessage,
} from '../types/event'
import { SystemStateMessage } from '../types/mtmsDevice'
import { MTMSEventMessage } from '../types/eeg'

const startSessionService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms_device/start_session',
  serviceType: 'mtms_device_interfaces/StartSession',
})
const stopSessionService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms_device/stop_session',
  serviceType: 'mtms_device_interfaces/StopSession',
})

const startDeviceService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms_device/start_device',
  serviceType: 'mtms_device_interfaces/StartDevice',
})
const stopDeviceService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms_device/stop_device',
  serviceType: 'mtms_device_interfaces/StopDevice',
})

export const systemStateSubscriber = new ROSLIB.Topic<SystemStateMessage>({
  ros: ros,
  name: '/mtms_device/system_state',
  messageType: 'mtms_device_interfaces/SystemState',
})

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

export const startSession = () => {
  const request = new ROSLIB.ServiceRequest()
  startSessionService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to start session')
      }
    },
    (error) => {
      console.log('ERROR: Failed to start session')
      console.error(error)
    },
  )
}

export const stopSession = () => {
  const request = new ROSLIB.ServiceRequest()
  stopSessionService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to stop session')
      }
    },
    (error) => {
      console.log('ERROR: Failed to stop session')
      console.error(error)
    },
  )
}

export const startDevice = () => {
  const request = new ROSLIB.ServiceRequest()
  startDeviceService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to start session')
      }
    },
    (error) => {
      console.log('ERROR: Failed to start session')
      console.error(error)
    },
  )
}

export const stopDevice = () => {
  const request = new ROSLIB.ServiceRequest()
  stopDeviceService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to stop experiment')
      }
    },
    (error) => {
      console.log('ERROR: Failed to stop experiment')
      console.error(error)
    },
  )
}
