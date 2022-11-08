import ROSLIB from 'roslib'
import { ros } from './ros'
import {
  ChargeFeedbackMessage,
  DischargeFeedbackMessage,
  PulseFeedbackMessage,
  SignalOutFeedbackMessage,
  SystemStateMessage
} from 'types/fpga'
import { MTMSEventMessage } from '../types/eeg'

const startExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/fpga/start_experiment',
  serviceType: 'fpga_interfaces/StartExperiment',
})
const stopExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/fpga/stop_experiment',
  serviceType: 'fpga_interfaces/StopExperiment',
})

const startDeviceService = new ROSLIB.Service({
  ros: ros,
  name: '/fpga/start_device',
  serviceType: 'fpga_interfaces/StartDevice',
})
const stopDeviceService = new ROSLIB.Service({
  ros: ros,
  name: '/fpga/stop_device',
  serviceType: 'fpga_interfaces/StopDevice',
})

export const systemStateSubscriber = new ROSLIB.Topic<SystemStateMessage>({
  ros: ros,
  name: '/fpga/system_state',
  messageType: 'fpga_interfaces/SystemState',
})

export const eventSubscriber = new ROSLIB.Topic<MTMSEventMessage>({
  ros: ros,
  name: '/mtms/events',
  messageType: 'mtms_interfaces/Event',
})

export const pulseFeedbackSubscriber = new ROSLIB.Topic<PulseFeedbackMessage>({
  ros: ros,
  name: '/fpga/pulse_feedback',
  messageType: 'fpga_interfaces/PulseFeedback',
})
export const chargeFeedbackSubscriber = new ROSLIB.Topic<ChargeFeedbackMessage>({
  ros: ros,
  name: '/fpga/charge_feedback',
  messageType: 'fpga_interfaces/ChargeFeedback',
})
export const dischargeFeedbackSubscriber = new ROSLIB.Topic<DischargeFeedbackMessage>({
  ros: ros,
  name: '/fpga/discharge_feedback',
  messageType: 'fpga_interfaces/DischargeFeedback',
})
export const signalOutFeedbackSubscriber = new ROSLIB.Topic<SignalOutFeedbackMessage>({
  ros: ros,
  name: '/fpga/signal_out_feedback',
  messageType: 'fpga_interfaces/SignalOutFeedback',
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
