from .enums.util.bcolors import bcolors

from fpga_interfaces.msg import DeviceState, ExperimentState, ExecutionCondition, StartupError

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

    def device_state_to_str(self, device_state):
        if device_state.value == device_state.NOT_OPERATIONAL:
            return self.colored_text("Not operational", "")

        elif device_state.value == device_state.STARTUP:
            return self.colored_text("Startup", bcolors.OKBLUE)

        elif device_state.value == device_state.OPERATIONAL:
            return self.colored_text("Operational", bcolors.OKGREEN)

        elif device_state.value == device_state.SHUTDOWN:
            return self.colored_text("Shutdown", bcolors.WARNING)

        else:
            assert False, "Invalid device state"

        return "{}{}{}".format(color, text, bcolors.ENDC)

    def experiment_state_to_str(self, experiment_state):
        if experiment_state.value == experiment_state.STOPPED:
            return self.colored_text("Stopped", bcolors.OKBLUE)

        elif experiment_state.value == experiment_state.STARTING:
            return self.colored_text("Starting", bcolors.OKBLUE)

        elif experiment_state.value == experiment_state.STARTED:
            return self.colored_text("Started", bcolors.OKGREEN)

        elif experiment_state.value == experiment_state.STOPPING:
            return self.colored_text("Stopping", bcolors.WARNING)

        else:
            assert False, "Invalid experiment state"

    def startup_error_to_str(self, startup_error):
        if startup_error.value == startup_error.NO_ERROR:
            return self.colored_text("No error", bcolors.OKGREEN)

        elif startup_error.value == startup_error.UART_INITIALIZATION_ERROR:
            return self.colored_text("UART initialization error", bcolors.FAIL)

        elif startup_error.value == startup_error.BOARD_STARTUP_ERROR:
            return self.colored_text("Board startup error", bcolors.FAIL)

        elif startup_error.value == startup_error.BOARD_STATUS_MESSAGE_ERROR:
            return self.colored_text("Board status message error", bcolors.FAIL)

        elif startup_error.value == startup_error.SAFETY_MONITOR_ERROR:
            return self.colored_text("Safety monitor error", bcolors.FAIL)

        elif startup_error.value == startup_error.DISCHARGE_CONTROLLER_ERROR:
            return self.colored_text("Discharge controller error", bcolors.FAIL)

        elif startup_error.value == startup_error.CHARGER_ERROR:
            return self.colored_text("Charger error", bcolors.FAIL)

        elif startup_error.value == startup_error.SENSORBOARD_ERROR:
            return self.colored_text("Sensorboard error", bcolors.FAIL)

        elif startup_error.value == startup_error.DISCHARGE_CONTROLLER_VOLTAGE_ERROR:
            return self.colored_text("Discharge controller voltage error", bcolors.FAIL)

        elif startup_error.value == startup_error.CHARGER_VOLTAGE_ERROR:
            return self.colored_text("Charger voltage error", bcolors.FAIL)

        elif startup_error.value == startup_error.IGBT_FEEDBACK_ERROR:
            return self.colored_text("IGBT feedback error", bcolors.FAIL)

        elif startup_error.value == startup_error.TEMPERATURE_SENSOR_PRESENCE_ERROR:
            return self.colored_text("Temparature sensor presence error", bcolors.FAIL)

        elif startup_error.value == startup_error.COIL_MEMORY_PRESENCE_ERROR:
            return self.colored_text("Coil memory presence error", bcolors.FAIL)

        else:
            assert False, "Invalid startup error"

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
        state_str = 'Device state: {}'.format(self.device_state_to_str(device_state))
        experiment_str = 'Experiment: {}'.format(self.experiment_state_to_str(experiment_state))
        startup_error_str = 'Startup error: {}'.format(self.startup_error_to_str(startup_error))

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

    def print_event(self, event_type, event, channel=None, port=None):
        assert channel is not None or port is not None

        execution_condition = event.execution_condition.value
        id = event.id
        time_us = event.time_us

        time_s = time_us / 10 ** 6

        if execution_condition == ExecutionCondition.TIMED:
            execution_condition_str = 'Timed at (s): {0:g}'.format(time_s)
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

    def print_feedback(self, event_type, error_enum, feedback):
        error_str = error_enum(feedback.status_code)
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
