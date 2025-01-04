import ROSLIB from 'roslib'
import { ros } from './ros'

import { EegBatchMessage, SampleMessage, EegTriggerMessage } from '../types/eeg'

/* Listener for EEG data */
export const eegDataSubscriber = new ROSLIB.Topic<EegBatchMessage>({
  ros: ros,
  name: '/eeg/batch_data',
  messageType: 'ui_interfaces/EegBatch',
})

/* Listener for trigger */
export const triggerSubscriber = new ROSLIB.Topic<EegTriggerMessage>({
  ros: ros,
  name: '/eeg/trigger',
  messageType: 'ui_interfaces/Trigger',
})
