import ROSLIB from 'roslib'
import { ros } from './ros'

import { PositionMessage } from '../types/target'

export const coilPositionSubscriber = new ROSLIB.Topic<PositionMessage>({
  ros: ros,
  name: '/neuronavigation/focus',
  messageType: 'neuronavigation_interfaces/PoseUsingEulerAngles',
})

/* Listener for coil at target */

/* TODO: Seems like this is not used */
const coilAtTargetListener = new ROSLIB.Topic({
  ros: ros,
  name: '/neuronavigation/coil_at_target',
  messageType: 'std_msgs/Bool',
})
//coilAtTargetListener.subscribe(updateCoilAtTarget);
