import math
import time

import rclpy

from rclpy.node import Node
from rclpy.qos import (
    QoSProfile,
    DurabilityPolicy,
    HistoryPolicy,
    ReliabilityPolicy,
)
from rclpy.executors import MultiThreadedExecutor
from rclpy.duration import Duration
from std_msgs.msg import String

from mtms_device_interfaces.msg import (
    ChannelError,
    SystemState,
    DeviceState,
    SessionState,
    SystemError,
    StartupError,
    ChannelState,
    Settings,
)
from mtms_device_interfaces.srv import (
    AllowStimulation,
    SendSettings,
    StartDevice,
    StopDevice,
    StopSession,
    StartSession,
)

from event_interfaces.msg import (
    Charge,
    ChargeError,
    Discharge,
    DischargeError,
    EventInfo,
    ExecutionCondition,
    Pulse,
    PulseError,
    TriggerOut,
    PulseFeedback,
    ChargeFeedback,
    DischargeFeedback,
    TriggerOutFeedback,
    WaveformPhase,
)


class MTMSSimulator(Node):
    """Simulator for mTMS device ros2 nodes.

    Simulates the mTMS device by replacing the mtms_device_bridge. Creates the corresponding nodes of the
    ros2 node interface described by the mtms_device_bridge packages.
    """

    SYSTEM_STATE_PUBLISHING_INTERVAL_MS = 20
    SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE_MS = 5

    # TODO: Make these values configurable
    CHARGE_RATE = 1 / 1500  # in J/s
    TIME_CONSTANT = 1  # in seconds, tau = RC
    PULSE_DISCHARGE_PERCENTAGE = 0.05

    MAX_VOLTAGE = 1500  # Volts
    NUM_OF_CHANNELS = 5

    def __init__(self):
        """
        Creates the ROS node and initializes the publishers, subscribes and the services.
        """

        super().__init__("mtms_simulator")

        # Services
        self.allow_stimulation_service = self.create_service(
            AllowStimulation,
            "/mtms_device/allow_stimulation",
            self.allow_stimulation_handler,
        )
        self.send_settings_service = self.create_service(
            SendSettings, "/mtms_device/send_settings", self.send_settings_handler
        )
        self.start_device_service = self.create_service(
            StartDevice, "/mtms_device/start_device", self.start_device_handler
        )
        self.stop_device_service = self.create_service(
            StopDevice, "/mtms_device/stop_device", self.stop_device_handler
        )
        self.start_session_service = self.create_service(
            StartSession, "/mtms_device/start_session", self.start_session_handler
        )
        self.stop_session_service = self.create_service(
            StopSession, "/mtms_device/stop_session", self.stop_session_handler
        )

        # Subscribers
        self.charge_subscriber = self.create_subscription(
            Charge, "/event/send/charge", self.charge_handler, 10
        )
        self.discharge_subscriber = self.create_subscription(
            Discharge, "/event/send/discharge", self.discharge_handler, 10
        )
        self.pulse_subscriber = self.create_subscription(
            Pulse, "/event/send/pulse", self.pulse_handler, 10
        )
        self.trigger_out_subscriber = self.create_subscription(
            TriggerOut, "event/send/trigger_out", self.trigger_out_handler, 10
        )

        # Publisher
        self.pulse_feedback_publisher = self.create_publisher(
            PulseFeedback, "/event/pulse_feedback", 10
        )
        self.charge_feedback_publisher = self.create_publisher(
            ChargeFeedback, "/event/charge_feedback", 10
        )
        self.discharge_feedback_publisher = self.create_publisher(
            DischargeFeedback, "/event/discharge_feedback", 10
        )
        self.trigger_out_feedback_publisher = self.create_publisher(
            TriggerOutFeedback, "/event/trigger_out_feedback", 10
        )
        self.node_message_publisher = self.create_publisher(String, "/node/message", 10)

        # QoS definition for System state.
        deadline_ns = 1000 * (
            MTMSSimulator.SYSTEM_STATE_PUBLISHING_INTERVAL_MS
            + MTMSSimulator.SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE_MS
        )
        lifespan_ns = deadline_ns

        system_state_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            deadline=Duration(nanoseconds=deadline_ns),
            lifespan=Duration(nanoseconds=lifespan_ns),
        )
        self.system_state_publisher = self.create_publisher(
            SystemState, "/mtms_device/system_state", system_state_qos
        )

        # Internal state variables
        self.system_state: SystemState = SystemState()
        self.system_state.channel_states = [
            ChannelState(channel_index=i) for i in range(self.NUM_OF_CHANNELS)
        ]

        self.session_start_time = 0.0
        self.allow_stimulation: bool = False
        self.settings: Settings = Settings()

        self.create_timer(
            MTMSSimulator.SYSTEM_STATE_PUBLISHING_INTERVAL_MS / 1000,
            self.system_state_callback,
        )

    def allow_stimulation_handler(self, request, response):
        self.allow_stimulation = request.allow_stimulation
        self.get_logger().info("Allow stimulation set to %r" % self.allow_stimulation)
        response.success = True
        return response

    def send_settings_handler(self, request, response):
        self.settings = request.settings
        self.get_logger().info("Received settings")
        self.get_logger().debug("Settings: %r" % self.settings)
        response.success = True
        return response

    def start_device_handler(self, request, response):
        # TODO: Check if STARTUP phase required
        self.system_state.device_state.value = DeviceState.OPERATIONAL
        self.get_logger().info("Device started")
        response.success = True
        return response

    def stop_device_handler(self, request, response):
        self.system_state.device_state.value = DeviceState.SHUTDOWN
        self.get_logger().info("Device stopped")
        response.success = True
        return response

    def start_session_handler(self, request, response):
        # TODO: Check if STARTING phase required
        if self.system_state.device_state.value != DeviceState.OPERATIONAL:
            self.get_logger().warn("Device not stated. Can't start session")
            return

        self.system_state.session_state.value = SessionState.STARTED
        self.session_start_time = time.time()
        self.get_logger().info("Session started")
        response.success = True
        return response

    def stop_session_handler(self, request, response):
        # TODO: Check if STOPPING phase required
        self.system_state.session_state.value = SessionState.STOPPED
        self.get_logger().info("Session stopped")
        response.success = True
        return response

    def charge_handler(self, message: Charge) -> None:
        """
        Charges the given channel to desired voltage.

        New voltage is not set immediately, and rather models the behaviour of a
        capacitor to change the voltage. Charging is done with constant power and the
        required energy to charge to certain voltage increases quadratically.

        Args:
            message: contains the charge information.
        """
        self.get_logger().info("Charge message received: %r" % message)

        # Session operational
        if self._check_session_started():
            self.get_logger().warn("Session not started. Can't charge coil.")
            return

        # Input validation
        error = self._validate_voltage_update_input(message)
        if error.value != ChargeError.NO_ERROR:
            feedback_msg = ChargeFeedback(id=message.event_info.id, error=error)
            self.charge_feedback_publisher.publish(feedback_msg)
            return

        # Wait for execution condition
        event_info: EventInfo = message.event_info
        self._wait_for_execution_execution_condition(
            event_info.execution_condition, event_info.execution_time
        )

        # Update voltage
        channel_state: ChannelState = self.system_state.channel_states[message.channel]

        current_voltage = channel_state.voltage
        target_voltage = message.target_voltage

        # TODO: Check the formula and constants
        t = self.CHARGE_RATE * (target_voltage**2 - current_voltage**2)
        time.sleep(t)
        channel_state.voltage = target_voltage

        # Send charge feedback after update ready
        feedback_msg = ChargeFeedback(id=message.event_info.id, error=error)
        self.charge_feedback_publisher.publish(feedback_msg)

    def discharge_handler(self, message: Discharge) -> None:
        """
        Discharges the given channel to given voltage.

        New voltage is not set immediately, and rather models the behaviour of a
        capacitor to change the voltage. Discharge follows exponential decay.

        Args:
            message: contains the discharge information.
        """
        self.get_logger().info("Discharge message received: %r" % message)

        # Session operational
        if self._check_session_started():
            self.get_logger().warn("Session not started. Can't discharge coil.")
            return

        # Input validation
        error = self._validate_voltage_update_input(message)
        if error.value != DischargeError.NO_ERROR:
            feedback_msg = DischargeFeedback(id=message.event_info.id, error=error)
            self.discharge_feedback_publisher.publish(feedback_msg)
            return

        # Wait for execution condition
        event_info: EventInfo = message.event_info
        self._wait_for_execution_execution_condition(
            event_info.execution_condition, event_info.execution_time
        )

        # Update voltage
        channel_state: ChannelState = self.system_state.channel_states[message.channel]

        current_voltage = channel_state.voltage
        target_voltage = message.target_voltage

        # TODO: Check the formula and constants
        t = self.TIME_CONSTANT * math.log(current_voltage / target_voltage)
        time.sleep(t)
        channel_state.voltage = target_voltage

        # Send charge feedback after update ready
        feedback_msg = DischargeFeedback(id=message.event_info.id, error=error)
        self.discharge_feedback_publisher.publish(feedback_msg)

    def _check_session_started(self) -> bool:
        return self.system_state.session_state.value != SessionState.STARTED

    def _validate_voltage_update_input(
        self, message: Charge | Discharge
    ) -> ChargeError | DischargeError | None:
        """Validate charge and discharge message input parameters

        Args:
            message: topic message containing charge or discharge information.
        Returns:
            Given error object containing the first encountered error.
        """

        match message:
            case Charge():
                error = ChargeError()
            case Discharge():
                error = DischargeError()
            case _:
                self.get_logger().error(
                    "Trying to validate voltage with wrong message type"
                )
                return None

        # TODO: Implement rest of the checks.
        if message.channel >= len(self.system_state.channel_states):
            self.get_logger().error(
                "Trying to charge invalid channel %d, configured channel count is %d"
                % message.channel,
                self.NUM_OF_CHANNELS,
            )
            error.value = ChargeError.INVALID_CHANNEL
            return error
        if message.target_voltage >= self.MAX_VOLTAGE:
            self.get_logger().error(
                "Too high voltage. Trying to set voltage to %d, max supported voltage is %d"
                % message.target_voltage,
                self.MAX_VOLTAGE,
            )
            error.value = ChargeError.INVALID_VOLTAGE
            return error
        return error

    def _wait_for_execution_execution_condition(
        self, execution_condition: ExecutionCondition, execution_time: float
    ) -> None:
        """
        Checks the execution condition and waits for the execution condition to be fulfilled.

        Args:
            execution_condition: will the execution be immediate or timed.
            execution_time: time when the event will be executed, relative to session start time.
        Returns:
            when execution condition is met.
        """

        # wait for execution condition.
        match execution_condition:
            case ExecutionCondition.TIMED:
                wait = execution_time - self.session_start_time
                time.sleep(wait)
            case ExecutionCondition.WAIT_FOR_TRIGGER:
                self.get_logger().warn(
                    "Execution condition WAIT_FOR_TRIGGER not supported. Doing nothing."
                )

    def pulse_handler(self, message: Pulse) -> None:
        """
        Simulates the behaviour of giving a pulse.

        Validates the pulse input and simulates the giving of a pulse by dropping
        the channel voltage by percentage value of PULSE_DISCHARGE_PERCENTAGE. Send
        pulse feedback message after execution.

        Args:
            message: the pulse information
        """
        self.get_logger().info("Pulse: %r" % message)

        # Validate input
        error = PulseError()
        if not self.allow_stimulation:
            self.get_logger().warn("Stimulation not allowed, skipping pulse.")
            return
        if message.channel >= len(self.system_state.channel_states):
            self.get_logger().warn("Invalid channel index: %d" % message.channel)
            error.value = PulseError.INVALID_CHANNEL
            return

        total_waveform_duration_in_ticks = 0
        rising_duration_in_ticks = 0
        falling_duration_in_ticks = 0
        for waveform_piece in message.waveform:
            # TODO: Check waveform phase type requirements.
            duration = waveform_piece.duration_in_ticks
            total_waveform_duration_in_ticks += duration
            match waveform_piece.waveform_phase:
                case WaveformPhase.RISING:
                    rising_duration_in_ticks += duration
                case WaveformPhase.FALLING:
                    falling_duration_in_ticks += duration

        if total_waveform_duration_in_ticks > self.settings.maximum_pulse_duration_ticks:
            error.value = PulseError.INVALID_DURATIONS

        rise_fall_diff = abs(rising_duration_in_ticks - falling_duration_in_ticks)
        if rise_fall_diff > self.settings.maximum_rising_falling_difference_ticks:
            error.value = PulseError.INVALID_DURATIONS

        # TODO: Implement pulse count limit checks.

        if error.value != PulseError.NO_ERROR:
            feedback_msg = PulseFeedback(id=message.event_info.id, error=error)
            self.pulse_feedback_publisher.publish(feedback_msg)
            return

        # Wait for execution condition
        event_info = EventInfo()
        self._wait_for_execution_execution_condition(
            event_info.execution_condition, event_info.execution_time
        )

        # Give pulse
        channel_state: ChannelState = self.system_state.channel_states[message.channel]

        channel_state.pulse_count += 1
        channel_state.voltage = (
            1 - self.PULSE_DISCHARGE_PERCENTAGE
        ) * channel_state.voltage

        # Send feedback
        feedback_msg = PulseFeedback(id=message.event_info.id, error=error)
        self.pulse_feedback_publisher.publish(feedback_msg)

    def trigger_out_handler(self, trigger_out):
        self.get_logger().info("Trigger out activate")
        self.get_logger().debug("Trigger out: %r" % trigger_out)

    # NOTE: System state is not published fast enough with python. The simulator might
    # need to be translated to cpp later.
    def system_state_callback(self):
        msg = self.system_state
        msg.time = 0.0
        if self.system_state.session_state.value == SessionState.STARTED:
            msg.time = time.time() - self.session_start_time  # in seconds
        self.system_state_publisher.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    mtms_simulator = MTMSSimulator()
    executor = MultiThreadedExecutor()
    rclpy.spin(mtms_simulator, executor=executor)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
