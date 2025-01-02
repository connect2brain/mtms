import ROSLIB from 'roslib'
import { ros } from './ros'

/* Get maximum intensity service */
const getMaximumIntensityService = new ROSLIB.Service({
  ros: ros,
  name: '/targeting/get_maximum_intensity',
  serviceType: 'targeting_interfaces/GetMaximumIntensity',
})

export const getMaximumIntensity = (
  x: number,
  y: number,
  angle: number,
  algorithm: number,
  callback: (maximum_intensity: number) => void
) => {
  const request = new ROSLIB.ServiceRequest({
    displacement_x: x,
    displacement_y: y,
    rotation_angle: angle,
    algorithm: {
      value: algorithm,
    },
  }) as any

  getMaximumIntensityService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to get maximum intensity')
      } else {
        callback(response.maximum_intensity)
      }
    },
    (error) => {
      console.log('ERROR: Failed to get maximum intensity, error:')
      console.log(error)
    }
  )
}
