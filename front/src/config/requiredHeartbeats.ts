/** Heartbeats the UI treats as required; all use std_msgs/msg/Empty. */
export const REQUIRED_MTMS_HEARTBEATS = [
  { id: 'busylight_manager', topic: '/mtms/busylight_manager/heartbeat', label: 'busylight_manager' },
  { id: 'experiment_performer', topic: '/mtms/experiment_performer/heartbeat', label: 'experiment_performer' },
  { id: 'pedal_listener', topic: '/mtms/pedal_listener/heartbeat', label: 'pedal_listener' },
  { id: 'trial_logger', topic: '/mtms/trial_logger/heartbeat', label: 'trial_logger' },
  { id: 'trial_validator', topic: '/mtms/trial_validator/heartbeat', label: 'trial_validator' },
  { id: 'voltage_setter', topic: '/mtms/voltage_setter/heartbeat', label: 'voltage_setter' },
] as const
