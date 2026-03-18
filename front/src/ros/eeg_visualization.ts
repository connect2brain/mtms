import ROSLIB from '@foxglove/roslibjs'
import { ros } from './ros'

import { EegBatchMessage, SampleMessage, EegTriggerMessage } from '../types/eeg'

/* Listener for EEG data */
export const eegDataSubscriber = new ROSLIB.Topic<EegBatchMessage>({
  ros: ros,
  name: '/eeg/batch_data',
  messageType: 'eeg_msgs/EegBatch',
})

/* Listener for trigger */
export const triggerSubscriber = new ROSLIB.Topic<EegTriggerMessage>({
  ros: ros,
  name: '/eeg/trigger',
  messageType: 'ui_interfaces/Trigger',
})
