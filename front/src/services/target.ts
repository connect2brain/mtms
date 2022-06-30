import { EulerAngles, Position, Target } from '../types/target'
import ROSLIB from 'roslib'
import { isOfTargetChangeableKey } from '../utils'
import { ros } from './ros'

/* Set up change_comment service. */
const changeCommentService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/change_comment',
  serviceType: 'mtms_interfaces/ChangeComment',
})
/* Set up add_target service. */
const addTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/add_target',
  serviceType: 'mtms_interfaces/AddTarget',
})
/* Set up rename_target service. */
const renameTargetService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/rename_target',
  serviceType: 'mtms_interfaces/RenameTarget',
})
/* Set up toggle_select service. */
const toggleTargetSelectService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_select_target',
  serviceType: 'mtms_interfaces/ToggleSelectTarget',
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

export const targetServicesByKey = {
  name: renameTargetService,
  comment: changeCommentService,
  visible: toggleVisibleService,
  selected: toggleTargetSelectService,
}

export const addTargetToRos = (position: Position, orientation: EulerAngles) => {
  const pose = new ROSLIB.Message({
    position,
    orientation,
  })

  const request = new ROSLIB.ServiceRequest({
    target: pose,
  })

  addTargetService.callService(
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

export const updateTargetInRos = (target: Target, key: string, value: any, toggle: boolean) => {
  if (!isOfTargetChangeableKey(key)) {
    console.error(`Key ${key} is not changeable`)
    return
  }

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

  targetServicesByKey[key].callService(
    request,
    (result) => {
      if (!result.success) {
        console.error(`ERROR: Failed to change key '${key}' from ${target[key]} to ${value}`)
      } else {
        console.log(`Changed ${target.name} key '${key}' from ${target[key]} to ${value}`)
      }
    },
    (error) => {
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
