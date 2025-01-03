import React from 'react'
import ROSLIB from 'roslib'

const ROS_URL =
  process.env.REACT_APP_ENV === 'dev' || process.env.REACT_APP_ENV === 'prod'
    ? 'ws://localhost:9090'
    : 'ws://rosbridge_test:9090'

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
