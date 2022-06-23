import { PulseSequence } from '../types/pulseSequence'
import ROSLIB from 'roslib'
import { ros } from './ros'

const removePulseService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_pulse',
  serviceType: 'mtms_interfaces/RemovePulse',
})
const setPulseIntensityService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_pulse',
  serviceType: 'mtms_interfaces/RemovePulse',
})
const setPulseIsiService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_pulse',
  serviceType: 'mtms_interfaces/RemovePulse',
})
const togglePulseSelectedService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_pulse',
  serviceType: 'mtms_interfaces/RemovePulse',
})


export const pulseServicesByKey = {
  intensity: setPulseIntensityService,
  isi: setPulseIsiService,
  selected: togglePulseSelectedService,
}

export const removePulseInRos = (pulseSequence: PulseSequence, pulseIndex: number) => {
  const request = new ROSLIB.ServiceRequest({
    name: pulseSequence.name,
    pulse_index: pulseIndex,
  })
  removePulseService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log(`ERROR: Failed to remove pulse of index ${pulseIndex} from ${pulseSequence}`)
      }
    },
    (error) => {
      console.log(`ERROR: Failed to remove pulse of index ${pulseIndex} from ${pulseSequence}, error:`)
      console.error(error)
    },
  )
}
