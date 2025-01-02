import ROSLIB from 'roslib'
import { ros } from './ros'

import { PositionMessage } from '../types/target'

export const coilPositionSubscriber = new ROSLIB.Topic<PositionMessage>({
  ros: ros,
  name: '/neuronavigation/focus',
  messageType: 'neuronavigation_interfaces/PoseUsingEulerAngles',
})
