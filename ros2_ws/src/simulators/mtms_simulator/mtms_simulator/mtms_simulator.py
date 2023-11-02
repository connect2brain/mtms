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
    TriggerOut,
    PulseFeedback,
    ChargeFeedback,
    DischargeFeedback,
    TriggerOutFeedback,
)


class MTMSSimulator(Node):
    """Simulator for mTMS device ros2 nodes.

    Simulates the mTMS device by replacing the mtms_device_bridge. Creates the corresponding nodes of the
    ros2 node interface described by the mtms_device_bridge packages.
    """

    SYSTEM_STATE_PUBLISHING_INTERVAL_MS = 20
    SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE_MS = 5

    # TODO: Make these values configurable
    CHARGE_RATE = 1500  # in J/s
    TIME_CONSTANT = 1  # in seconds, tau = RC

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
        event_info: EventInfo = message.event_info
        charge_error = ChargeError()

        error = self._validate_voltage_update_input(message, charge_error)
        self._wait_for_execution_execution_condition(
            event_info.execution_condition, event_info.execution_time
        )

        channel_state: ChannelState = self.system_state.channel_states[message.channel]

        current_voltage = channel_state.voltage
        target_voltage = message.target_voltage

        t = self.CHARGE_RATE * (target_voltage**2 - current_voltage**2)
        time.sleep(t)

        channel_state.voltage = target_voltage

        feedback_msg = ChargeFeedback(id=message.event_info.id, error=error)
        self.charge_feedback_publisher.publish(feedback_msg)

    def discharge_handler(self, message) -> None:
        """
        Charges or discharges the given channel to given voltage.

        New voltage is not set immediately, and rather models the behaviour of a
        capacitor to change the voltage. Discharge
        follows exponential decay.

        Args:
            message: contains the discharge information.

        Returns:
        """
        event_info: EventInfo = message.event_info
        discharge_error = DischargeError()

        error = self._validate_voltage_update_input(message, discharge_error)
        self._wait_for_execution_execution_condition(
            event_info.execution_condition, event_info.execution_time
        )

        channel_state: ChannelState = self.system_state.channel_states[message.channel]

        current_voltage = channel_state.voltage
        target_voltage = message.target_voltage

        t = self.TIME_CONSTANT * math.log(current_voltage / target_voltage)
        time.sleep(t)
        channel_state.voltage = target_voltage

        feedback_msg = DischargeFeedback(id=message.event_info.id, error=error)

        self.discharge_feedback_publisher.publish(feedback_msg)

    def _validate_voltage_update_input(
        self, message: Charge | Discharge, error: ChargeError | DischargeError
    ) -> ChargeError | DischargeError:
        """Validate charge and discharge message input parameters

        Args:
            message: topic message containing charge or discharge information.
            error: object containing the error value.
        Returns:
            Given error object containing the first encountered error.
        """
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

    def pulse_handler(self, message):
        self.get_logger().info("Pulse: %r" % message)

    def trigger_out_handler(self, trigger_out):
        self.get_logger().info("Trigger out activate")
        self.get_logger().debug("Trigger out: %r" % trigger_out)

    def system_state_callback(self):
        msg = self.system_state
        msg.time = 0.0
        if self.system_state.session_state.value == SessionState.STARTED:
            msg.time = time.time() - self.session_start_time  # in seconds
        self.system_state_publisher.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    mtms_simulator = MTMSSimulator()
    executor = MultiThreadedExecutor(num_threads=4)
    rclpy.spin(mtms_simulator, executor=executor)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
