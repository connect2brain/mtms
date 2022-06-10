import ROSLIB from 'roslib'

export interface EulerAngles {
  alpha: number
  beta: number
  gamma: number
}

export interface Position {
  x: number
  y: number
  z: number
}

export interface PoseUsingEulerAngles {
  orientation: EulerAngles
  position: Position
}

export interface Target {
  name: string
  type: string
  comment: string
  visible: boolean
  selected: boolean
  target: boolean
  pose: PoseUsingEulerAngles

  intensity: number
  iti: number
}

//type ChangeableKeys = keyof Target
export const changeableKey = ['name', 'comment', 'visible', 'selected'] as const
export type ChangeableKey = typeof changeableKey[number]

export interface TargetMessage extends ROSLIB.Message {
  targets: Target[]
}

export interface PositionMessage extends ROSLIB.Message {
  position: Position
  orientation: EulerAngles
}
