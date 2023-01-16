from util.bcolors import bcolors

from fpga_interfaces.msg import DeviceState, ExperimentState, StartupError
from event_interfaces.msg import ExecutionCondition, PulseError, DischargeError, ChargeError, SignalOutError

class MTMSApiEnums():
    DEVICE_STATES = (
        (DeviceState.NOT_OPERATIONAL, "Not operational", ""),
        (DeviceState.STARTUP, "Startup", bcolors.OKBLUE),
        (DeviceState.OPERATIONAL, "Operational", bcolors.OKGREEN),
        (DeviceState.SHUTDOWN, "Shutdown", bcolors.WARNING),
    )
    EXPERIMENT_STATES = (
        (ExperimentState.STOPPED, "Stopped", bcolors.OKBLUE),
        (ExperimentState.STARTING, "Starting", bcolors.OKBLUE),
        (ExperimentState.STARTED, "Started", bcolors.OKGREEN),
        (ExperimentState.STOPPING, "Stopping", bcolors.WARNING),
    )
    STARTUP_ERRORS = (
        (StartupError.NO_ERROR, "No error", bcolors.OKGREEN),
        (StartupError.UART_INITIALIZATION_ERROR, "UART initialization error", bcolors.FAIL),
        (StartupError.BOARD_STARTUP_ERROR, "Board startup error", bcolors.FAIL),
        (StartupError.BOARD_STATUS_MESSAGE_ERROR, "Board status message error", bcolors.FAIL),
        (StartupError.SAFETY_MONITOR_ERROR, "Safety monitor error", bcolors.FAIL),
        (StartupError.DISCHARGE_CONTROLLER_ERROR, "Discharge controller error", bcolors.FAIL),
        (StartupError.CHARGER_ERROR, "Charger error", bcolors.FAIL),
        (StartupError.SENSORBOARD_ERROR, "Sensorboard error", bcolors.FAIL),
        (StartupError.DISCHARGE_CONTROLLER_VOLTAGE_ERROR, "Discharge controller voltage error", bcolors.FAIL),
        (StartupError.CHARGER_VOLTAGE_ERROR, "Charger voltage error", bcolors.FAIL),
        (StartupError.IGBT_FEEDBACK_ERROR, "IGBT feedback error", bcolors.FAIL),
        (StartupError.TEMPERATURE_SENSOR_PRESENCE_ERROR, "Temperature sensor presence error", bcolors.FAIL),
        (StartupError.COIL_MEMORY_PRESENCE_ERROR, "Coil memory presence error", bcolors.FAIL),
    )
    PULSE_ERRORS = (
        (PulseError.NO_ERROR, "No error", bcolors.OKGREEN),
        (PulseError.INVALID_EXECUTION_CONDITION, "Invalid execution condition", bcolors.FAIL),
        (PulseError.INVALID_CHANNEL, "Invalid channel", bcolors.FAIL),
        (PulseError.INVALID_NUMBER_OF_WAVEFORM_PIECES, "Invalid number of pieces", bcolors.FAIL),
        (PulseError.INVALID_MODES, "Invalid modes", bcolors.FAIL),
        (PulseError.INVALID_DURATIONS, "Invalid durations", bcolors.FAIL),
        (PulseError.LATE, "Late", bcolors.FAIL),
        (PulseError.TOO_MANY_PULSES, "Too many pulses", bcolors.FAIL),
        (PulseError.OVERLAPPING_WITH_CHARGING, "Overlapping with charging", bcolors.FAIL),
        (PulseError.OVERLAPPING_WITH_DISCHARGING, "Overlapping with discharging", bcolors.FAIL),
        (PulseError.TRIGGERING_FAILURE, "Triggering failure", bcolors.FAIL),
        (PulseError.UNKNOWN_ERROR, "Unknown error", bcolors.FAIL),
    )
    CHARGE_ERRORS = (
        (ChargeError.NO_ERROR, "No error", bcolors.OKGREEN),
        (ChargeError.INVALID_EXECUTION_CONDITION, "Invalid execution condition", bcolors.FAIL),
        (ChargeError.INVALID_CHANNEL, "Invalid channel", bcolors.FAIL),
        (ChargeError.INVALID_VOLTAGE, "Invalid voltage", bcolors.FAIL),
        (ChargeError.LATE, "Late", bcolors.FAIL),
        (ChargeError.OVERLAPPING_WITH_DISCHARGING, "Overlapping with discharging", bcolors.FAIL),
        (ChargeError.OVERLAPPING_WITH_STIMULATION, "Overlapping with stimulation", bcolors.FAIL),
        (ChargeError.CHARGING_FAILURE, "Charging failure", bcolors.FAIL),
        (ChargeError.UNKNOWN_ERROR, "Unknown error", bcolors.FAIL),
    )
    DISCHARGE_ERRORS = (
        (DischargeError.NO_ERROR, "No error", bcolors.OKGREEN),
        (DischargeError.INVALID_EXECUTION_CONDITION, "Invalid execution condition", bcolors.FAIL),
        (DischargeError.INVALID_CHANNEL, "Invalid channel", bcolors.FAIL),
        (DischargeError.INVALID_VOLTAGE, "Invalid voltage", bcolors.FAIL),
        (DischargeError.LATE, "Late", bcolors.FAIL),
        (DischargeError.OVERLAPPING_WITH_CHARGING, "Overlapping with charging", bcolors.FAIL),
        (DischargeError.OVERLAPPING_WITH_STIMULATION, "Overlapping with stimulation", bcolors.FAIL),
        (DischargeError.DISCHARGING_FAILURE, "Discharging failure", bcolors.FAIL),
        (DischargeError.UNKNOWN_ERROR, "Unknown error", bcolors.FAIL),
    )
    SIGNAL_OUT_ERRORS = (
        (SignalOutError.NO_ERROR, "No error", bcolors.OKGREEN),
        (SignalOutError.INVALID_EXECUTION_CONDITION, "Invalid execution condition", bcolors.FAIL),
        (SignalOutError.LATE, "Late", bcolors.FAIL),
        (SignalOutError.SIGNALOUT_FAILURE, "Signal out failure", bcolors.FAIL),
        (SignalOutError.UNKNOWN_ERROR, "Unknown error", bcolors.FAIL),
    )


class MTMSApiPrinter():
    TIME_COLOR = bcolors.OKBLUE
    EVENT_COLORS = {
        'Pulse': bcolors.HEADER,
        'Charge': bcolors.OKCYAN,
        'Discharge': bcolors.WARNING,
        'Signal out': bcolors.OKGREEN,
    }
    def __init__(self):

        # TODO: Hard-coded channel count and temperature and pulse count support for now.
        self.n_channels = 5
        self.support_temperature = True
        self.support_pulse_count = True

        self.print_every_nth_system_state = 5
        self.system_state_counter = 0

    def colored_text(self, text, color):
        return "{}{}{}".format(color, text, bcolors.ENDC)

    def enum_to_str(self, value, enums):
        for enum, text, color in enums:
            if value == enum:
                return self.colored_text(text, color)

        assert False, "Invalid value"

    def print_system_state(self, state):
        self.system_state_counter += 1
        if self.system_state_counter != self.print_every_nth_system_state:
            return

        self.system_state_counter = 0

        voltages_str = 'V: '
        temperatures_str = 'T: '
        pulse_counts_str = 'P: '

        for channel in range(self.n_channels):
            channel_state = state.channel_states[channel]

            voltage = '{:5}'.format(channel_state.voltage)
            voltages_str += voltage

            temperature = '{:4}'.format(channel_state.temperature)
            temperatures_str += temperature

            pulse_count = '{:4}'.format(channel_state.pulse_count)
            pulse_counts_str += pulse_count

        startup_error = state.startup_error
        device_state = state.device_state
        experiment_state = state.experiment_state

        time_str = '{}Time (s){}: {:.2f}'.format(self.TIME_COLOR, bcolors.ENDC, state.time)
        state_str = 'Device state: {}'.format(self.enum_to_str(device_state.value, MTMSApiEnums.DEVICE_STATES))
        experiment_str = 'Experiment: {}'.format(self.enum_to_str(experiment_state.value, MTMSApiEnums.EXPERIMENT_STATES))
        startup_error_str = 'Startup error: {}'.format(self.enum_to_str(startup_error.value, MTMSApiEnums.STARTUP_ERRORS))

        status_str = ', '.join(filter(None, [
            time_str,
            voltages_str,
            temperatures_str if self.support_temperature else None,
            pulse_counts_str if self.support_pulse_count else None,
            state_str,
            experiment_str,
            startup_error_str if startup_error != StartupError.NO_ERROR else None,
        ]))
        print(status_str)

    def print_event(self, event_type, event_info, channel=None, port=None):
        assert channel is not None or port is not None

        execution_condition = event_info.execution_condition.value
        id = event_info.id
        time = event_info.execution_time

        if execution_condition == ExecutionCondition.TIMED:
            execution_condition_str = 'Timed at (s): {0:g}'.format(time)
        elif execution_condition == ExecutionCondition.TRIGGER:
            execution_condition_str = 'Waiting for trigger'
        elif execution_condition == ExecutionCondition.INSTANT:
            execution_condition_str = 'Instant'
        else:
            assert False, "Unknown execution condition"

        print('{}[Sent] {:10.10s} {}  {:7.7s}: {}  Event ID: {}, {}'.format(
            self.EVENT_COLORS[event_type],
            event_type,
            bcolors.ENDC,
            "Channel" if channel is not None else "Port",
            channel if channel is not None else port,
            id,
            execution_condition_str
        ))

    def print_feedback(self, event_type, feedback):
        value = feedback.error.value

        if event_type == "Pulse":
            error_str = self.enum_to_str(value, MTMSApiEnums.PULSE_ERRORS)
        elif event_type == "Charge":
            error_str = self.enum_to_str(value, MTMSApiEnums.CHARGE_ERRORS)
        elif event_type == "Discharge":
            error_str = self.enum_to_str(value, MTMSApiEnums.DISCHARGE_ERRORS)
        elif event_type == "Signal out":
            error_str = self.enum_to_str(value, MTMSApiEnums.SIGNAL_OUT_ERRORS)

        print('{}[Done] {:10.10s} {}  {:7.7s}     Event ID: {}, Status: {}'.format(
            self.EVENT_COLORS[event_type],
            event_type,
            bcolors.ENDC,
            "",
            feedback.id,
            error_str
        ))

    def print_event_trigger(self):
        # HACK: This should probably be a feedback message that is received from the FPGA, informing that the
        #   event trigger was successfully generated, similar to the feedback messages from the actual events.
        #
        print('{}Event trigger{}'.format(bcolors.OKGREEN, bcolors.ENDC))

    def print_heading(self, text):
        print('')
        print('{}{}{}{}{}{}'.format("", bcolors.HEADER, "\033[1m", text, "\033[0m", bcolors.ENDC))
