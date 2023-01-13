from .enums.util.bcolors import bcolors

from .enums.DeviceState import DeviceState
from .enums.ExperimentState import ExperimentState
from .enums.ExecutionCondition import ExecutionCondition
from .enums.StartupSequenceError import StartupSequenceError

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

        startup_sequence_error = StartupSequenceError(state.startup_sequence_error)
        device_state = DeviceState(state.device_state)
        experiment_state = ExperimentState(state.experiment_state)

        time_str = '{}Time (s){}: {:.2f}'.format(self.TIME_COLOR, bcolors.ENDC, state.time)
        state_str = 'Device state: {}'.format(device_state)
        experiment_str = 'Experiment: {}'.format(experiment_state)
        startup_sequence_error_str = 'Startup sequence error: {}'.format(startup_sequence_error)

        status_str = ', '.join(filter(None, [
            time_str,
            voltages_str,
            temperatures_str if self.support_temperature else None,
            pulse_counts_str if self.support_pulse_count else None,
            state_str,
            experiment_str,
            startup_sequence_error_str if startup_sequence_error != StartupSequenceError.NO_ERROR else None,
        ]))
        print(status_str)

    def print_event(self, event_type, event, channel=None, port=None):
        assert channel is not None or port is not None

        execution_condition = ExecutionCondition(event.execution_condition)
        id = event.id
        time_us = event.time_us

        time_s = time_us / 10 ** 6

        if execution_condition == ExecutionCondition.TIMED:
            execution_condition_str = 'Timed at (s): {0:g}'.format(time_s)
        elif execution_condition == ExecutionCondition.TRIGGERED:
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

    def print_heading(self, text):
        print('')
        print('{}{}{}{}{}{}'.format("", bcolors.HEADER, "\033[1m", text, "\033[0m", bcolors.ENDC))
