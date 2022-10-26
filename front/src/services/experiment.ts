import ROSLIB from 'roslib'
import { ros } from './ros'
import { SystemStateMessage } from 'types/fpga'
import { MTMSEventMessage } from '../types/eeg'

const startExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/start_experiment',
  serviceType: 'fpga_interfaces/StartExperiment',
})
const stopExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/stop_experiment',
  serviceType: 'fpga_interfaces/StopExperiment',
})

const startDeviceService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/start_device',
  serviceType: 'fpga_interfaces/StartDevice',
})
const stopDeviceService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/stop_device',
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
