import math
import time

from mtms_device_interfaces.msg import ChannelError

from event_interfaces.msg import (
    EventInfo,
    ChargeFeedback,
    ChargeError,
    DischargeFeedback,
    DischargeError,
    PulseError,
    PulseFeedback,
)


class Channel:
    """mTMS simulator coil channel.

    Responsible for simulating the voltage charge and discharge and pulses.
    """

    CLOCK_FREQUENCY_HZ = 4e7

    def __init__(
        self,
        charge_rate: float,
        time_constant: float,
        pulse_discharge_percentage: float,
        max_voltage: int,
    ):
        """
        Initialising of the Channel with channel specific properties.

        Args:
            charge_rate: voltage charging rate in J/s
            time_constant: discharge rate time constant in seconds tau=RC
            pulse_discharge_percentage: the percentage amount which the voltage drops per pulse
            max_voltage: the max voltage of this channel.
        """

        self.charge_rate = charge_rate
        """Charge rate of the voltage in volts."""

        self.time_constant = time_constant
        """Unit less constant related to voltage discharge."""

        self.pulse_discharge_percentage = pulse_discharge_percentage
        """Voltage drop in percentages when pulse is given."""

        self.max_voltage = max_voltage
        """max voltage of the coil."""

        # Internal state

        self.current_voltage = 0
        """Current voltage in volts"""

        self.temperature = 24
        """Current temperature in C"""

        self.pulse_count = 0
        """Session pulse count total"""

        self.errors = ChannelError()
        """Channel error states"""

        self.is_charging = False
        """True when charging."""

        self.is_discharging = False
        """True when discharging."""

        self.is_pulse_in_progress = False
        """True when giving a pulse"""

    def charge(self, target_voltage: int, event_info: EventInfo) -> ChargeFeedback:
        """
        Charge the channel to desired voltage.

        New voltage is not set immediately, and rather models the behaviour of a
        capacitor to change the voltage. Charging is done with constant power and the
        required energy to charge to certain voltage increases quadratically.

        Args:
            target_voltage: the new voltage to charge to in volts.
            event_info: the received charge event info.

        Returns:
            After charging is ready returns with ChargeFeedback with event id and errors.
        """
        t = 1 / 2 * self.charge_rate * (target_voltage**2 - self.current_voltage**2)
        self.is_charging = True
        time.sleep(t)
        self.current_voltage = target_voltage
        self.is_charging = False

        return ChargeFeedback(id=event_info.id, error=ChargeError.NO_ERROR)

    def discharge(
        self, target_voltage: int, event_info: EventInfo
    ) -> DischargeFeedback:
        """
        Discharges the channel to desired voltage.

        New voltage is not set immediately, and rather models the behaviour of a
        capacitor to change the voltage. Discharge follows exponential decay.

        Args:
            target_voltage: the new voltage to discharge to in volts.
            event_info: the received charge event info.

        Returns:
            After discharging is ready returns with DischargeFeedback with event id and errors.
        """
        t = self.time_constant * math.log(self.current_voltage / target_voltage)
        self.is_discharging = True
        time.sleep(t)
        self.current_voltage = target_voltage
        self.is_discharging = False

        return DischargeFeedback(id=event_info.id, error=DischargeError.NO_ERROR)

    def pulse(self, event_id: int, duration_ticks: int) -> PulseFeedback:
        """
        Simulates the behaviour of giving a pulse.

        Validates the pulse input and simulates the giving of a pulse by dropping
        the channel voltage by percentage value of PULSE_DISCHARGE_PERCENTAGE. Send
        pulse feedback message after execution.

        Args:
            event_id: id of the event.
            duration_ticks: pulse duration in ticks.
        """
        self.pulse_count += 1
        duration_seconds = duration_ticks / Channel.CLOCK_FREQUENCY_HZ

        self.is_pulse_in_progress = True
        new_voltage_percentage = 1 - self.pulse_discharge_percentage
        time.sleep(duration_seconds)
        self.current_voltage = new_voltage_percentage * self.current_voltage
        self.is_pulse_in_progress = False

        return PulseFeedback(id=event_id, error=PulseError.NO_ERROR)
