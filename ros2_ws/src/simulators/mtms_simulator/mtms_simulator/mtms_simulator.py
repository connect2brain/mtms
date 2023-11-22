import time

import rclpy
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
    WaveformPiece,
)
from mtms_device_interfaces.msg import (
    SystemState,
    DeviceState,
    SessionState,
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
from rcl_interfaces.msg import ParameterDescriptor, ParameterType
from rclpy.duration import Duration
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node
from rclpy.qos import (
    QoSProfile,
    DurabilityPolicy,
    HistoryPolicy,
    ReliabilityPolicy,
)
from std_msgs.msg import String

from .channel import Channel


class MTMSSimulator(Node):
    """Simulator for mTMS device ros2 nodes.

    Simulates the mTMS device by replacing the mtms_device_bridge. Creates the
    corresponding topics of the ros2 node interface described by the mtms_device_bridge
    nodes."""

    SYSTEM_STATE_PUBLISHING_INTERVAL_MS = 20
    SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE_MS = 5

    # TODO: Make these values configurable
    CAPACITANCE = 1020e-6
    """Capacitance of the coils in farads."""

    TIME_CONSTANT = 1  # tau = RC
    """Time constant related to coil discharging in seconds"""

    PULSE_VOLTAGE_DROP_PROPORTION = 0.05
    """The proportion in which the voltage of the channel drops"""

    def __init__(self):
        """
        Creates the ROS node and initializes the publishers, subscribes and the services.
        """

        super().__init__("mtms_simulator")

        descriptor = ParameterDescriptor(
            name="Number of mTMS channels",
            type=ParameterType.PARAMETER_INTEGER,
        )
        self.declare_parameter("channels", value=5, descriptor=descriptor)
        self.num_of_channels = self.get_parameter("channels").value
        self.get_logger().info("Number of channels: %d" % self.num_of_channels)

        descriptor = ParameterDescriptor(
            name="Maximum voltage of the coils in volts",
            type=ParameterType.PARAMETER_INTEGER,
        )
        self.declare_parameter("max_voltage", value=1500, descriptor=descriptor)
        self.max_voltage = self.get_parameter("max_voltage").value

        descriptor = ParameterDescriptor(
            name="Charge rate of the coils in J/s", type=ParameterType.PARAMETER_INTEGER
        )
        self.declare_parameter("charge_rate", value=1500, descriptor=descriptor)
        self.charge_rate = self.get_parameter("charge_rate").value

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
        self.system_state.channel_states = self.num_of_channels * [None]

        self.channels = [
            Channel(
                charge_rate=self.charge_rate,
                capacitance=self.CAPACITANCE,
                time_constant=self.TIME_CONSTANT,
                pulse_voltage_drop_proportion=self.PULSE_VOLTAGE_DROP_PROPORTION,
                max_voltage=self.max_voltage,
            )
            for _ in range(self.num_of_channels)
        ]

        self.session_start_time = 0.0
        self.allow_stimulation: bool = False
        self.settings: Settings = Settings()

        self.create_timer(
            MTMSSimulator.SYSTEM_STATE_PUBLISHING_INTERVAL_MS / 1000,
            self.system_state_callback,
        )

    def allow_stimulation_handler(self, request, response):
        self.get_logger().info(
            "Allow stimulation set to %r" % request.allow_stimulation
        )

        self.allow_stimulation = request.allow_stimulation
        for channel in self.channels:
            channel.allow_stimulation = request.allow_stimulation

        response.success = True
        return response

    def send_settings_handler(self, request, response):
        self.get_logger().info("Received settings")
        self.get_logger().debug("Settings: %r" % request.settings)

        self.settings = request.settings
        for channel in self.channels:
            channel.settings = self.settings

        response.success = True
        return response

    def start_device_handler(self, request, response):
        # NOTE: skipping STARTUP phase
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
        # NOTE: Skipping STARTING phase
        if self.system_state.device_state.value != DeviceState.OPERATIONAL:
            self.get_logger().warn("Device not started. Can't start session")
            return

        self.system_state.session_state.value = SessionState.STARTED
        self.session_start_time = time.time()
        self.get_logger().info("Session started")
        response.success = True
        return response

    def stop_session_handler(self, request, response):
        # NOTE: Skipping STOPPING phase
        self.system_state.session_state.value = SessionState.STOPPED
        self.get_logger().info("Session stopped")
        response.success = True
        return response

    def _is_session_started(self) -> bool:
        return self.system_state.session_state.value != SessionState.STARTED

    def _validate_charge_or_discharge(
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
                self.num_of_channels,
            )
            error.value = ChargeError.INVALID_CHANNEL
            return error
        if message.target_voltage >= self.max_voltage:
            self.get_logger().error(
                "Too high voltage. Trying to set voltage to %d, maximum supported voltage is %d"
                % message.target_voltage,
                self.max_voltage,
            )
            error.value = ChargeError.INVALID_VOLTAGE
            return error
        return error

    def _wait_for_execution_condition(
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
        match execution_condition.value:
            case ExecutionCondition.TIMED:
                wait = execution_time - (time.time() - self.session_start_time)
                time.sleep(wait)
            case ExecutionCondition.WAIT_FOR_TRIGGER:
                self.get_logger().warn(
                    "Execution condition WAIT_FOR_TRIGGER not supported. Doing nothing."
                )
            case ExecutionCondition.IMMEDIATE:
                pass  # Immediate, do nothing
            case _:
                self.get_logger().warn(
                    "Invalid execution condition: %r, assuming IMMEDIATE"
                    % ExecutionCondition.value
                )

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

        # Check if session operational
        if self._is_session_started():
            self.get_logger().warn("Session not started. Can't charge coil.")
            return

        # Input validation
        error = self._validate_charge_or_discharge(message)
        if error.value != ChargeError.NO_ERROR:
            feedback_msg = ChargeFeedback(id=message.event_info.id, error=error)
            self.charge_feedback_publisher.publish(feedback_msg)
            return

        # Wait for execution condition
        event_info: EventInfo = message.event_info
        self._wait_for_execution_condition(
            execution_condition=event_info.execution_condition,
            execution_time=event_info.execution_time,
        )

        # Update voltage
        channel = self.channels[message.channel]
        feedback_msg = channel.charge(message.target_voltage, message.event_info)
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
        if self._is_session_started():
            self.get_logger().warn("Session not started. Can't discharge coil.")
            return

        # Input validation
        error = self._validate_charge_or_discharge(message)
        if error.value != DischargeError.NO_ERROR:
            feedback_msg = DischargeFeedback(id=message.event_info.id, error=error)
            self.discharge_feedback_publisher.publish(feedback_msg)
            return

        # Wait for execution condition
        event_info: EventInfo = message.event_info
        self._wait_for_execution_condition(
            execution_condition=event_info.execution_condition,
            execution_time=event_info.execution_time,
        )

        # Update voltage
        channel = self.channels[message.channel]
        feedback_msg = channel.discharge(message.target_voltage, message.event_info)
        self.discharge_feedback_publisher.publish(feedback_msg)

    def _validate_pulse(self, message: Pulse) -> PulseError:
        """
        Validates the pulse message.

        Args:
            message: the message containing pulse information.

        Returns:
            First encountered error or no error if no errors.
        """

        # General errors
        if not self.allow_stimulation:
            self.get_logger().warn("Stimulation not allowed, skipping pulse.")
            return PulseError(value=PulseError.NOT_ALLOWED)

        if message.channel >= len(self.system_state.channel_states):
            self.get_logger().warn("Invalid channel index: %d" % message.channel)
            return PulseError(value=PulseError.INVALID_CHANNEL)

        # TODO: Implement execution condition check for other types as well.
        execution_condition = message.event_info.execution_condition
        if execution_condition not in (
            ExecutionCondition.IMMEDIATE,
            ExecutionCondition.TIMED,
            ExecutionCondition.WAIT_FOR_TRIGGER,
        ):
            return PulseError(value=PulseError.INVALID_EXECUTION_CONDITION)

        # Channel specific errors
        channel = self.channels[message.channel]

        if channel.is_charging:
            return PulseError(value=PulseError.OVERLAPPING_WITH_CHARGING)
        if channel.is_discharging:
            return PulseError(value=PulseError.OVERLAPPING_WITH_DISCHARGING)

        # Waveform related errors
        (
            total_duration,
            rising_duration,
            falling_duration,
        ) = self._calculate_waveform_durations(waveform=message.waveform)
        if total_duration > self.settings.maximum_pulse_duration_ticks:
            return PulseError(value=PulseError.INVALID_DURATIONS)

        rise_fall_diff = abs(rising_duration - falling_duration)
        if rise_fall_diff > self.settings.maximum_rising_falling_difference_ticks:
            return PulseError(value=PulseError.INVALID_DURATIONS)

        return PulseError(value=PulseError.NO_ERROR)

    def _calculate_waveform_durations(
        self, waveform: list[WaveformPiece]
    ) -> (int, int, int):
        """
        Calculated the waveform durations

        Args:
            waveform: the pulse waveform

        Returns:
            total, rising and falling duration in ticks respectively.
        """
        total_duration = 0
        rising_duration = 0
        falling_duration = 0
        for waveform_piece in waveform:
            # TODO: Check waveform phase type requirements.
            duration = waveform_piece.duration_in_ticks
            total_duration += duration
            match waveform_piece.waveform_phase:
                case WaveformPhase.RISING:
                    rising_duration += duration
                case WaveformPhase.FALLING:
                    falling_duration += duration

        return total_duration, rising_duration, falling_duration

    def pulse_handler(self, message: Pulse) -> None:
        """
        Simulates the behaviour of giving a pulse.

        Simulates the giving of a pulse by dropping the channel voltage by the amount of
        self.pulse_drop_proportion.

        Args:
            message: the pulse information
        """
        self.get_logger().info("Pulse: %r" % message)

        # Wait for execution condition
        event_info = message.event_info
        self._wait_for_execution_condition(
            execution_condition=event_info.execution_condition,
            execution_time=event_info.execution_time,
        )

        # Validate pulse
        error = self._validate_pulse(message)
        if error.value != PulseError.NO_ERROR:
            feedback_msg = PulseFeedback(id=message.event_info.id, error=error)
            self.pulse_feedback_publisher.publish(feedback_msg)
            return

        # Give pulse
        channel = self.channels[message.channel]
        total_duration, _, _ = self._calculate_waveform_durations(message.waveform)
        feedback_msg = channel.pulse(event_info.id, total_duration)
        self.pulse_feedback_publisher.publish(feedback_msg)

    def trigger_out_handler(self, message: TriggerOut):
        self.get_logger().debug("Trigger out: %r" % message)

        event_info = message.event_info
        self._wait_for_execution_condition(
            execution_condition=event_info.execution_condition,
            execution_time=event_info.execution_time,
        )

        # Wait for the duration
        time.sleep(message.duration_us / 1e6)

        feedback_msg = TriggerOutFeedback(id=event_info.id)
        self.trigger_out_feedback_publisher.publish(feedback_msg)

    # NOTE: System state is not published fast enough with python. The simulator might
    # need to be translated to cpp later.
    def system_state_callback(self):
        msg = self.system_state

        msg.time = 0.0
        if msg.session_state.value == SessionState.STARTED:
            msg.time = time.time() - self.session_start_time  # in seconds

        for i in range(self.num_of_channels):
            channel = self.channels[i]
            channel_state = ChannelState(
                channel_index=i,
                voltage=channel.current_voltage,
                temperature=channel.temperature,
                pulse_count=channel.pulse_count,
                channel_error=channel.errors,
            )
            msg.channel_states[i] = channel_state

        self.system_state_publisher.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    mtms_simulator = MTMSSimulator()
    executor = MultiThreadedExecutor()
    rclpy.spin(mtms_simulator, executor=executor)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
