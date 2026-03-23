import ROSLIB from '@foxglove/roslibjs'

import { ros } from './ros'

export interface ElectricTarget {
  displacement_x: number
  displacement_y: number
  rotation_angle: number
  intensity: number
  algorithm: number
}

const startRemoteControllerService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/remote_controller/start',
  serviceType: 'mtms_trial_interfaces/StartRemoteController',
})

const stopRemoteControllerService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/remote_controller/stop',
  serviceType: 'std_srvs/Trigger',
})

export const startRemoteController = (targetLists: ElectricTarget[][], onSuccess?: () => void) => {
  const request = new ROSLIB.ServiceRequest({
    target_lists: targetLists.map((targets) => ({ targets })),
  })
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
