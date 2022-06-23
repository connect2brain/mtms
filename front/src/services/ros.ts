import ROSLIB from 'roslib'
import { ROS_URL } from '../utils/constants'
import { EulerAngles, Position, PositionMessage, Target, TargetMessage } from '../types/target'
import { isOfPulseSequenceChangeableKey, isOfTargetChangeableKey, objectKeysToSnakeCase } from '../utils'
import { Pulse, PulseSequence } from '../types/pulseSequence'

export const ros = new ROSLIB.Ros({
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

export const defaultOptions = {
  //latch: true,
  //queue_length: 1,
  //queue_size: 1,
}

export const positionListener = new ROSLIB.Topic<PositionMessage>({
  ros: ros,
  name: '/neuronavigation/focus',
  messageType: 'neuronavigation_interfaces/PoseUsingEulerAngles',
  ...defaultOptions,
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
  ...defaultOptions,
})
//coilAtTargetListener.subscribe(updateCoilAtTarget);

/* Set up listener for planner state. */
export const stateListener = new ROSLIB.Topic<TargetMessage>({
  ros: ros,
  name: '/planner/state',
  messageType: 'mtms_interfaces/PlannerState',
  ...defaultOptions,
})

/* Set up start_experiment service */
export const startExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/stimulation/start_experiment',
  serviceType: 'mtms_interfaces/ToggleNavigation',
})

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
