import ROSLIB from 'roslib'
import { ros } from './ros'
import {
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  PulseFeedbackMessage,
  SignalOutFeedbackMessage,
} from '../types/event'
import { SystemStateMessage } from '../types/mtmsDevice'
import { MTMSEventMessage } from '../types/eeg'

const startExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms_device/start_experiment',
  serviceType: 'mtms_device_interfaces/StartExperiment',
})
const stopExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms_device/stop_experiment',
  serviceType: 'mtms_device_interfaces/StopExperiment',
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
export const signalOutFeedbackSubscriber = new ROSLIB.Topic<SignalOutFeedbackMessage>({
  ros: ros,
  name: '/event/signal_out_feedback',
  messageType: 'event_interfaces/SignalOutFeedback',
})

export const startExperiment = () => {
  const request = new ROSLIB.ServiceRequest()
  startExperimentService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to start experiment')
      }
    },
    (error) => {
      console.log('ERROR: Failed to start experiment')
      console.error(error)
    },
  )
}

export const stopExperiment = () => {
  const request = new ROSLIB.ServiceRequest()
  stopExperimentService.callService(
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

export const startDevice = () => {
  const request = new ROSLIB.ServiceRequest()
  startDeviceService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to start experiment')
      }
    },
    (error) => {
      console.log('ERROR: Failed to start experiment')
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
