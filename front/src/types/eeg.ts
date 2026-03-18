import ROSLIB from '@foxglove/roslibjs'

export interface EegDeviceInfoMessage extends ROSLIB.Message {
  is_streaming: boolean
  sampling_frequency: number
  num_eeg_channels: number
  num_emg_channels: number
}
