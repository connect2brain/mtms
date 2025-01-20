import time
from enum import Enum

import numpy as np
from nifpga import Session

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class Boards(Enum):
    SENSORBOARD = 0
    CHARGER_INTERFACE = 1
    SAFETY_MONITOR = 2
    DISCHARGE_CONTROLLER = 3

class BoardsApi:
    SENSORBOARD_QUERY_DELAY_S = 0.1
    CHARGER_INTERFACE_QUERY_DELAY_S = 0.1
    N_DECIMALS = 5
    N_CHANNELS = 5
    BOARD_STARTUP_DELAY_S = 13

    def __init__(self, filename, reset=False):
        if reset:
            self.session = Session(filename, "RIO0")
            self.session.reset()

        self.session = Session(filename, "RIO0")
        self.memory_presences = None
        self.sensor_presences = None

    def set_power(self, new_state):
        old_state = self.session.registers['+15 V main'].read()
        self.session.registers['+15 V main'].write(new_state)

        if new_state and not old_state:
            print("Waiting for board startup", end="", flush=True)
            for i in range(self.BOARD_STARTUP_DELAY_S):
                time.sleep(1)
                print(".", end="", flush=True)

    def get_suffix(self, board):
        if board == Boards.SENSORBOARD:
            return '(SB)'
        elif board == Boards.CHARGER_INTERFACE:
            return '(CI)'
        elif board == Boards.SAFETY_MONITOR:
            return '(SM)'
        elif board == Boards.DISCHARGE_CONTROLLER:
            return '(DC)'
        else:
            assert False, "Unknown board"

    def get_register_value(self, register_name, board):
        register_name_suffixed = register_name + " " + self.get_suffix(board)
        value = self.session.registers[register_name_suffixed].read()
        return value

    def print_register_value(self, register_name, board, value, coil=None, channel=None, print_hex=False):
        if coil is not None:
            print('{}{} (coil {}){}: {}: {}'.format(bcolors.HEADER, board.name.lower(), coil, bcolors.ENDC, register_name, value if not print_hex else hex(value)))
        elif channel is not None:
            print('{}{} (channel {}){}: {}: {}'.format(bcolors.HEADER, board.name.lower(), channel, bcolors.ENDC, register_name, value if not print_hex else hex(value)))
        else:
            if type(value) is float:
                value = round(value, self.N_DECIMALS)

            print('{}{}{}: {}: {}'.format(bcolors.HEADER, board.name.lower(), bcolors.ENDC, register_name, value if not print_hex else hex(value)))

    def print_version(self, board, major, minor, patch, channel=None):
        if channel is not None:
            print('{}{} (channel {}){}: Version: {}.{}.{}'.format(bcolors.HEADER, board.name.lower(), channel, bcolors.ENDC, major, minor, patch))
        else:
            print('{}{}{}: Version: {}.{}.{}'.format(bcolors.HEADER, board.name.lower(), bcolors.ENDC, major, minor, patch))

    def print_heading(self, text):
        print('')
        print('{}{}{}{}{}{}'.format("", bcolors.HEADER, "\033[1m", text, "\033[0m", bcolors.ENDC))

    def print_register_list(self, register_name, board, values):
        s = ', '.join(str(e) for e in values)
        print('{}{}{}: {}: {}'.format(bcolors.HEADER, board.name.lower(), bcolors.ENDC, register_name, s))

    # Generic
    def get_sensorboard_value(self, command, register_name, coil=None, channel=None, print_hex=False, extra_delay=0):
        assert channel is None or coil is None, "Cannot query for both coil and channel at the same time."

        if coil is not None:
            self.session.registers['Connector (SB)'].write(coil)

        if channel is not None:
            self.session.registers['Channel (SB)'].write(channel)

        self.session.registers['Command (SB)'].write(command)
        time.sleep(self.SENSORBOARD_QUERY_DELAY_S)
        time.sleep(extra_delay)

        value = self.get_register_value(register_name, Boards.SENSORBOARD)
        self.print_register_value(register_name, Boards.SENSORBOARD, value, coil=coil, channel=channel, print_hex=print_hex)

    def get_charger_interface_value(self, command, register_name, print_hex=False):
        self.session.registers['Command (SB)'].write(command)
        time.sleep(self.SENSORBOARD_QUERY_DELAY_S)

        value = self.get_register_value(register_name, Boards.CHARGER_INTERFACE)
        self.print_register_value(register_name, Boards.CHARGER_INTERFACE, value)

    # Board versions
    def get_sensorboard_version(self):
        command = 0xF0
        self.session.registers['Command (SB)'].write(command)
        time.sleep(self.SENSORBOARD_QUERY_DELAY_S)

        major = self.get_register_value('Version major', Boards.SENSORBOARD)
        minor = self.get_register_value('Version minor', Boards.SENSORBOARD)
        patch = self.get_register_value('Version patch', Boards.SENSORBOARD)
        self.print_version(Boards.SENSORBOARD, major, minor, patch)

        return major, minor, patch

    def get_charger_interface_version(self):
        command = 0x70
        self.session.registers['Command (CI)'].write(command)
        time.sleep(self.CHARGER_INTERFACE_QUERY_DELAY_S)

        major = self.get_register_value('Version major', Boards.CHARGER_INTERFACE)
        minor = self.get_register_value('Version minor', Boards.CHARGER_INTERFACE)
        patch = self.get_register_value('Version patch', Boards.CHARGER_INTERFACE)
        self.print_version(Boards.CHARGER_INTERFACE, major, minor, patch)

        return major, minor, patch

    def get_safety_monitor_version(self):
        major = self.get_register_value('Version major', Boards.SAFETY_MONITOR)
        minor = self.get_register_value('Version minor', Boards.SAFETY_MONITOR)
        patch = self.get_register_value('Version patch', Boards.SAFETY_MONITOR)
        self.print_version(Boards.SAFETY_MONITOR, major, minor, patch)

        return major, minor, patch

    def get_discharge_controller_version(self, channel):
        major = self.get_register_value('Channel {}, Version major'.format(channel + 1), Boards.DISCHARGE_CONTROLLER)
        minor = self.get_register_value('Channel {}, Version minor'.format(channel + 1), Boards.DISCHARGE_CONTROLLER)
        patch = self.get_register_value('Channel {}, Version patch'.format(channel + 1), Boards.DISCHARGE_CONTROLLER)
        self.print_version(Boards.DISCHARGE_CONTROLLER, major, minor, patch, channel=channel)

        return major, minor, patch

    # Sensorboard
    def get_sensor_presences(self):
        command = 0x20
        self.session.registers['Command (SB)'].write(command)
        time.sleep(self.SENSORBOARD_QUERY_DELAY_S)

        register_name = 'Sensor presences'
        raw_presences = self.get_register_value(register_name, Boards.SENSORBOARD)
        sensor_presences = np.where(raw_presences)[0]
        self.print_register_list(register_name, Boards.SENSORBOARD, sensor_presences)

        return sensor_presences

    def get_memory_presences(self):
        command = 0x21
        self.session.registers['Command (SB)'].write(command)
        time.sleep(self.SENSORBOARD_QUERY_DELAY_S)

        register_name = 'Memory presences'
        raw_presences = self.get_register_value(register_name, Boards.SENSORBOARD)
        memory_presences = np.where(raw_presences)[0]
        self.print_register_list(register_name, Boards.SENSORBOARD, memory_presences)

        return memory_presences

    def get_transducer_id(self, coil):
        register_name = 'Transducer ID'
        command = 0x30
        return self.get_sensorboard_value(command, register_name, coil=coil, print_hex=True)

    def get_coil_type(self, coil):
        register_name = 'Coil type'
        command = 0x31
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_coil_layer(self, coil):
        register_name = 'Coil layer'
        command = 0x32
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_coil_inductance(self, coil):
        register_name = 'Coil inductance'
        command = 0x33
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_coil_resistance(self, coil):
        register_name = 'Coil resistance'
        command = 0x34
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_threshold_for_temperature_warning(self, coil):
        register_name = 'Threshold for temperature warning'
        command = 0x35
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_threshold_for_critical_temperature(self, coil):
        register_name = 'Threshold for critical temperature'
        command = 0x36
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_temperature(self, coil):
        register_name = 'Temperature'
        command = 0x41
        return self.get_sensorboard_value(command, register_name, coil=coil, extra_delay=1.0)

    def get_pulse_count(self, coil):
        register_name = 'Pulse count'
        command = 0x50
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_maximum_pulse_count(self, coil):
        register_name = 'Maximum pulse count'
        command = 0x51
        return self.get_sensorboard_value(command, register_name, coil=coil)

    def get_device_id(self):
        register_name = 'Device ID'
        command = 0x60
        return self.get_sensorboard_value(command, register_name, print_hex=True)

    def get_charger_power(self):
        register_name = 'Charger power'
        command = 0x61
        return self.get_sensorboard_value(command, register_name)

    def get_channel_capacitance(self, channel):
        register_name = 'Channel capacitance'
        command = 0x70
        return self.get_sensorboard_value(command, register_name, channel=channel)

    def get_channel_discharge_resistance(self, channel):
        register_name = 'Channel discharge resistance'
        command = 0x71
        return self.get_sensorboard_value(command, register_name, channel=channel)

    # Charger interface
    def get_setpoint_correction(self):
        command = 0x50

        register_name = 'Setpoint correction, slope'
        slope = self.get_charger_interface_value(command, register_name)

        register_name = 'Setpoint correction, offset'
        offset = self.get_charger_interface_value(command, register_name)

        return slope, offset

    def get_measurement_correction(self):
        command = 0x51

        register_name = 'Measurement correction, slope'
        slope = self.get_charger_interface_value(command, register_name)

        register_name = 'Measurement correction, offset'
        offset = self.get_charger_interface_value(command, register_name)

        return slope, offset

    # Board infos
    def print_sensorboard_info(self):
        self.print_heading("Sensorboard")
        self.get_sensorboard_version()

        self.print_heading("General")
        self.get_device_id()

        self.sensor_presences = self.get_sensor_presences()
        self.memory_presences = self.get_memory_presences()

        for coil in self.memory_presences:
            self.print_heading("Coil {}".format(coil))

            self.get_transducer_id(coil)
            self.get_coil_type(coil)
            self.get_coil_layer(coil)
            self.get_coil_inductance(coil)
            self.get_coil_resistance(coil)
            self.get_threshold_for_temperature_warning(coil)
            self.get_threshold_for_critical_temperature(coil)
            self.get_pulse_count(coil)
            self.get_maximum_pulse_count(coil)

        self.print_heading("Channel capacitance")
        for channel in range(self.N_CHANNELS):
            self.get_channel_capacitance(channel)

        self.print_heading("Channel discharge resistance")
        for channel in range(self.N_CHANNELS):
            self.get_channel_discharge_resistance(channel)

    def print_charger_interface_info(self):
        self.print_heading("Charger interface")
        self.get_charger_interface_version()

        self.print_heading("Voltage correction")
        self.get_setpoint_correction()
        self.get_measurement_correction()

    def print_safety_monitor_info(self):
        self.print_heading("Safety monitor")
        self.get_safety_monitor_version()

    def print_discharge_controller_info(self):
        self.print_heading("Discharge controller")
        for channel in range(self.N_CHANNELS):
            self.get_discharge_controller_version(channel)

# XXX: Should work with this bitfile, as long as it's found by the script, but untested as of Jan 2025.
api = BoardsApi("NiFpga_mTMS_Tubingen_Board_control_0.1.3.lvbitx")

api.set_power(True)

api.print_safety_monitor_info()
api.print_discharge_controller_info()
api.print_charger_interface_info()
api.print_sensorboard_info()

# for coil in api.sensor_presences:
#     print("")
#     temperature = api.get_temperature(coil)
