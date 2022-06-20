import ROSLIB from 'roslib'
import { ROS_URL } from '../utils/constants'
import { EulerAngles, Position, PositionMessage, Target, TargetMessage } from '../types/target'
import { isOfChangeableKey, objectKeysToSnakeCase } from '../utils'
import { Pulse, PulseSequence } from '../types/pulseSequence'

const ros = new ROSLIB.Ros({
  url: ROS_URL,
})

ros.on('connection', () => {
  console.log('ROS connected to', ROS_URL)
})

ros.on('error', (error) => {
  console.log('Error connecting ROS to', ROS_URL, ',error:', error)
})

ros.on('close', () => {
  console.log('ROS closed ws connection')
})

export const positionListener = new ROSLIB.Topic<PositionMessage>({
  ros: ros,
  name: '/neuronavigation/focus',
  messageType: 'neuronavigation_interfaces/PoseUsingEulerAngles',
})

/* Set up add_target service. */
const addTargetClient = new ROSLIB.Service({
  ros: ros,
  name: '/planner/add_target',
  serviceType: 'mtms_interfaces/AddTarget',
})

/* Set up add_target service. */
const addPulseSequenceClient = new ROSLIB.Service({
  ros: ros,
  name: '/planner/add_pulse_sequence',
  serviceType: 'mtms_interfaces/AddPulseSequence',
})

/* Set up toggle_select service. */
const toggleSelectService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_select',
  serviceType: 'mtms_interfaces/ToggleSelect',
})

/* Set up rename_target service. */
const renameTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/rename_target',
  serviceType: 'mtms_interfaces/RenameTarget',
})

/* Set up remove_target service. */
const removeTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_target',
  serviceType: 'mtms_interfaces/RemoveTarget',
})

const changeTargetIndexService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/change_target_index',
  serviceType: 'mtms_interfaces/ChangeTargetIndex',
})

/* Set up set_target service. */
const setTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/set_target',
  serviceType: 'mtms_interfaces/SetTarget',
})

/* Set up toggle_visible service. */
const toggleVisibleService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_visible',
  serviceType: 'mtms_interfaces/ToggleVisible',
})

/* Set up change_comment service. */
const changeCommentService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/change_comment',
  serviceType: 'mtms_interfaces/ChangeComment',
})

/* Set up toggle_navigation service.*/
const toggleNavigationService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_navigation',
  serviceType: 'mtms_interfaces/ToggleNavigation',
})

const clearStateService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/clear_state',
  serviceType: 'mtms_interfaces/ClearState',
})

/* Set up listener for coil at target. */
const coilAtTargetListener = new ROSLIB.Topic({
  ros: ros,
  name: '/neuronavigation/coil_at_target',
  messageType: 'std_msgs/Bool',
})
//coilAtTargetListener.subscribe(updateCoilAtTarget);

/* Set up listener for planner state. */
export const stateListener = new ROSLIB.Topic<TargetMessage>({
  ros: ros,
  name: '/planner/state',
  messageType: 'mtms_interfaces/PlannerState',
})

/* Set up start_experiment service */
export const startExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/stimulation/start_experiment',
  serviceType: 'mtms_interfaces/ToggleNavigation',
})

const rosServicesByKey = {
  name: renameTargetService,
  comment: changeCommentService,
  visible: toggleVisibleService,
  selected: toggleSelectService,
}

export const addTargetToRos = (position: Position, orientation: EulerAngles) => {
  const pose = new ROSLIB.Message({
    position,
    orientation,
  })

  const request = new ROSLIB.ServiceRequest({
    target: pose,
  })

  addTargetClient.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to add target', pose)
      }
    },
    (error) => {
      console.log('ERROR: Failed to add target', pose, ', error:')
      console.error(error)
    },
  )
}

export const changeTargetIndexInRos = (target: Target, newIndex: number) => {
  const request = new ROSLIB.ServiceRequest({
    name: target.name,
    new_index: newIndex,
  })
  changeTargetIndexService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to change target', target, 'index')
      }
    },
    (error) => {
      console.log('ERROR: Failed to change target', target, 'index, error')
      console.error(error)
    },
  )
}

export const addPulseSequenceToRos = (pulses: Pulse[]) => {
  const snakeCasePulses = objectKeysToSnakeCase({
    pulses,
  })

  console.log(snakeCasePulses)

  const request = new ROSLIB.ServiceRequest({
    pulses: snakeCasePulses.pulses,
  })

  addPulseSequenceClient.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to add pulse sequence')
      }
    },
    (error) => {
      console.log('ERROR: Failed to add pulse sequence, error:')
      console.error(error)
    },
  )
}

export const clearRosState = () => {
  const request = new ROSLIB.ServiceRequest()
  console.log('clearing ros state')
  clearStateService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to clear state')
      }
    },
    (error) => {
      console.log('ERROR: Failed to clear state, error:')
      console.error(error)
    },
  )
}

export const removeTargetInRos = (target: Target) => {
  const request = new ROSLIB.ServiceRequest({
    name: target.name,
  })
  removeTargetService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to remove target', target)
      }
    },
    (error) => {
      console.log('ERROR: Failed to remove target', target, ', error:')
      console.error(error)
    },
  )
}

export const updateTargetInRos = (target: Target, key: string, value: any, toggle: boolean, targets: Target[]) => {
  if (!isOfChangeableKey(key)) {
    console.error(`Key ${key} is not changeable`)
    return
  }

  const newTarget = {
    ...target,
    [key]: value,
  }
  const newTargets = [...targets]
  newTargets[targets.indexOf(target)] = newTarget

  let requestObject = {
    name: target.name,
  }
  if (!toggle) {
    const requestKey = `new_${key}`
    requestObject = {
      ...requestObject,
      [requestKey]: value,
    }
  }
  const request = new ROSLIB.ServiceRequest(requestObject)

  rosServicesByKey[key].callService(
    request,
    (result) => {
      if (!result.success) {
        console.error(`ERROR: Failed to change key '${key}' from ${target[key]} to ${value}`)
      } else {
        console.log(`Changed ${target.name} key '${key}' from ${target[key]} to ${value}`)
        //setTargets(newTargets)
      }
    },
    (error) => {
      console.error(error)
    },
  )
}
