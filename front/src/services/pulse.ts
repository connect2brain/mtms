import { PulseSequence } from '../types/pulseSequence'
import ROSLIB from 'roslib'
import { ros } from './ros'
import { isOfPulseChangeableKey, isOfPulseSequenceChangeableKey } from '../utils'
import { pulseSequenceServicesByKey } from './pulseSequence'

const removePulseService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_pulse',
  serviceType: 'mtms_interfaces/RemovePulse',
})
const setPulseIntensityService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/set_pulse_intensity',
  serviceType: 'mtms_interfaces/SetPulseIntensity',
})
const setPulseIsiService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/set_pulse_isi',
  serviceType: 'mtms_interfaces/SetPulseIsi',
})
const togglePulseSelectedService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_select_pulse',
  serviceType: 'mtms_interfaces/ToggleSelectPulse',
})

const togglePulseVisibleService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_visible_pulse',
  serviceType: 'mtms_interfaces/ToggleVisiblePulse',
})

export const pulseServicesByKey = {
  intensity: setPulseIntensityService,
  isi: setPulseIsiService,
  selected: togglePulseSelectedService,
  visible: togglePulseVisibleService,
}

export const updatePulseInRos = (
  sequence: PulseSequence,
  pulseIndex: number,
  key: string,
  value: any,
  toggle: boolean,
) => {
  console.log(sequence, pulseIndex, key, value, toggle)
  if (!isOfPulseChangeableKey(key)) {
    console.error(`Key ${key} is not changeable`)
    return
  }

  let requestObject = {
    name: sequence.name,
    pulse_index: pulseIndex,
  }
  if (!toggle) {
    const requestKey = `new_${key}`
    requestObject = {
      ...requestObject,
      [requestKey]: value,
    }
  }

  console.log(requestObject)
  const request = new ROSLIB.ServiceRequest(requestObject)

  pulseServicesByKey[key].callService(
    request,
    (result) => {
      if (!result.success) {
        console.error(`ERROR: Failed to change pulse key '${key}' from ${sequence.pulses[pulseIndex][key]} to ${value}`)
      } else {
        console.log(
          `Changed ${sequence.name} pulse index ${pulseIndex} key '${key}' from ${sequence.pulses[pulseIndex][key]} to ${value}`,
        )
      }
    },
    (error) => {
      console.error(error)
    },
  )
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
