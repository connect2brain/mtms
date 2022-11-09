export const StartupError = {
  NO_ERROR: 0,
  UART_INITIALIZATION_ERROR: 1,
  BOARD_STARTUP_ERROR: 2,
  BOARD_STATUS_MESSAGE_ERROR: 3,
  SAFETY_MONITOR_ERROR: 4,
  DISCHARGE_CONTROLLER_ERROR: 5,
  CHARGER_ERROR: 6,
  SENSORBOARD_ERROR: 7,
  DISCHARGE_CONTROLLER_VOLTAGE_ERROR: 8,
  CHARGER_VOLTAGE_ERROR: 9,
  IGBT_FEEDBACK_ERROR: 10,
  TEMPERATURE_SENSOR_PRESENCE_ERROR: 11,
  COIL_MEMORY_PRESENCE_ERROR: 12,
}

export const ChargeError = {
  NO_ERROR: 0,
  INVALID_EXECUTION_CONDITION: 1,
  INVALID_CHANNEL: 2,
  INVALID_VOLTAGE: 3,
  LATE: 4,
  OVERLAPPING_WITH_DISCHARGING: 5,
  OVERLAPPING_WITH_STIMULATION: 6,
  CHARGING_FAILURE: 7,
  UNKNOWN_ERROR: 8,
}

export const PulseError = {
  NO_ERROR: 0,
  INVALID_EXECUTION_CONDITION: 1,
  INVALID_CHANNEL: 2,
  INVALID_NUMBER_OF_WAVEFORM_PIECES: 3,
  INVALID_MODES: 4,
  INVALID_DURATIONS: 5,
  LATE: 6,
  TOO_MANY_PULSES: 7,
  OVERLAPPING_WITH_CHARGING: 8,
  OVERLAPPING_WITH_DISCHARGING: 9,
  TRIGGERING_FAILURE: 10,
  UNKNOWN_ERROR: 11,
}

export const SignalOutError = {
  NO_ERROR: 0,
  INVALID_EXECUTION_CONDITION: 1,
  LATE: 2,
  SIGNALOUT_FAILURE: 3,
  UNKNOWN_ERROR: 4,
}

export const DischargeError = {
  NO_ERROR: 0,
  INVALID_EXECUTION_CONDITION: 1,
  INVALID_CHANNEL: 2,
  INVALID_VOLTAGE: 3,
  LATE: 4,
  OVERLAPPING_WITH_CHARGING: 5,
  OVERLAPPING_WITH_STIMULATION: 6,
  DISCHARGING_FAILURE: 7,
  UNKNOWN_ERROR: 8,
}

export const errorsByType = {
  pulse: PulseError,
  signalOut: SignalOutError,
  charge: ChargeError,
  discharge: DischargeError,
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
