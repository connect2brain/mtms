export interface StartupError {
  NO_ERROR: 0
  UART_INITIALIZATION_ERROR: 1
  BOARD_STARTUP_ERROR: 2
  BOARD_STATUS_MESSAGE_ERROR: 3
  SAFETY_MONITOR_ERROR: 4
  DISCHARGE_CONTROLLER_ERROR: 5
  CHARGER_ERROR: 6
  SENSORBOARD_ERROR: 7
  DISCHARGE_CONTROLLER_VOLTAGE_ERROR: 8
  CHARGER_VOLTAGE_ERROR: 9
  IGBT_FEEDBACK_ERROR: 10
  TEMPERATURE_SENSOR_PRESENCE_ERROR: 11
  COIL_MEMORY_PRESENCE_ERROR: 12
  value: number
}

export interface ChargeError {
  value: number
  NO_ERROR: number
  INVALID_EXECUTION_CONDITION: number
  INVALID_CHANNEL: number
  INVALID_VOLTAGE: number
  LATE: number
  OVERLAPPING_WITH_DISCHARGING: number
  OVERLAPPING_WITH_STIMULATION: number
  CHARGING_FAILURE: number
  UNKNOWN_ERROR: number
}

export interface PulseError {
  value: number
  NO_ERROR: number
  INVALID_EXECUTION_CONDITION: number
  INVALID_CHANNEL: number
  INVALID_NUMBER_OF_WAVEFORM_PIECES: number
  INVALID_MODES: number
  INVALID_DURATIONS: number
  LATE: number
  TOO_MANY_PULSES: number
  OVERLAPPING_WITH_CHARGING: number
  OVERLAPPING_WITH_DISCHARGING: number
  TRIGGERING_FAILURE: number
  UNKNOWN_ERROR: number
}

export interface SignalOutError {
  value: number
  NO_ERROR: number
  INVALID_EXECUTION_CONDITION: number
  LATE: number
  SIGNALOUT_FAILURE: number
  UNKNOWN_ERROR: number
}

export interface DischargeError {
  value: number
  NO_ERROR: number
  INVALID_EXECUTION_CONDITION: number
  INVALID_CHANNEL: number
  INVALID_VOLTAGE: number
  LATE: number
  OVERLAPPING_WITH_CHARGING: number
  OVERLAPPING_WITH_STIMULATION: number
  DISCHARGING_FAILURE: number
  UNKNOWN_ERROR: number
}

export interface SystemErrorMessage {
  heartbeat_error: boolean
  latched_fault_error: boolean
  powersupply_error: boolean
  safety_bus_error: boolean
  coil_error: boolean
  emergency_button_error: boolean
  door_error: boolean
  charger_overvoltage_error: boolean
  charger_overtemperature_error: boolean
  monitored_voltage_over_maximum_error: boolean
  patient_safety_error: boolean
  device_safety_error: boolean
  charger_powerup_error: boolean
  opto_error: boolean
  charger_power_enabled_twice_error: boolean
}

export interface ChannelErrorMessage {
  overvoltage_error: boolean
  emergency_discharge_backup_power_error: boolean
  safety_bus_error: boolean
  powersupply_error: boolean
  safety_bus_startup_error: boolean
  acceptable_voltage_not_reached_startup_error: boolean
  maximum_safe_voltage_exceeded_startup_error: boolean
}
