import ROSLIB from 'roslib'
import { ROS_URL } from '../utils/constants'
import { ChangeableKey, EulerAngles, Position, PositionMessage, Target, TargetMessage } from '../types/target'
import { ExperimentMessage } from '../types/pulseSequence'
import useStore from '../providers/state'
import { isOfChangeableKey } from '../utils'

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

export const positionListener = new ROSLIB.Topic<PositionMessage>({
  ros: ros,
  name: '/neuronavigation/focus',
  messageType: 'neuronavigation_interfaces/PoseUsingEulerAngles',
})

//listener.subscribe(updatePosition);

/* Set up add_target service. */
export const addTargetClient = new ROSLIB.Service({
  ros: ros,
  name: '/planner/add_target',
  serviceType: 'mtms_interfaces/AddTarget',
})

/* Set up toggle_select service. */
export const toggleSelectService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_select',
  serviceType: 'mtms_interfaces/ToggleSelect',
})

/* Set up rename_target service. */
export const renameTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/rename_target',
  serviceType: 'mtms_interfaces/RenameTarget',
})

/* Set up remove_target service. */
export const removeTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/remove_target',
  serviceType: 'mtms_interfaces/RemoveTarget',
})

/* Set up set_target service. */
export const setTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/set_target',
  serviceType: 'mtms_interfaces/SetTarget',
})

/* Set up toggle_visible service. */
export const toggleVisibleService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_visible',
  serviceType: 'mtms_interfaces/ToggleVisible',
})

/* Set up change_comment service. */
export const changeCommentService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/change_comment',
  serviceType: 'mtms_interfaces/ChangeComment',
})

/* Set up toggle_navigation service.*/
export const toggleNavigationService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_navigation',
  serviceType: 'mtms_interfaces/ToggleNavigation',
})

/* Set up listener for coil at target. */
export const coilAtTargetListener = new ROSLIB.Topic({
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

export const updateTargetInRos = (
  target: Target,
  key: string,
  value: any,
  toggle: boolean,
  targets: Target[],
  setTargets: any,
) => {
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

