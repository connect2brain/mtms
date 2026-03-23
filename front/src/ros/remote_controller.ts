import ROSLIB from '@foxglove/roslibjs'

import { ros } from './ros'

const startRemoteControllerService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/remote_controller/start',
  serviceType: 'std_srvs/Trigger',
})

const stopRemoteControllerService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/remote_controller/stop',
  serviceType: 'std_srvs/Trigger',
})

export const startRemoteController = (onSuccess?: () => void) => {
  const request = new ROSLIB.ServiceRequest({})
  startRemoteControllerService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to start remote controller')
        return
      }
      onSuccess?.()
    },
    (error: any) => {
      console.log('ERROR: Failed to start remote controller')
      console.error(error)
    }
  )
}

export const stopRemoteController = (onSuccess?: () => void) => {
  const request = new ROSLIB.ServiceRequest({})
  stopRemoteControllerService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to stop remote controller')
        return
      }
      onSuccess?.()
    },
    (error: any) => {
      console.log('ERROR: Failed to stop remote controller')
      console.error(error)
    }
  )
}

