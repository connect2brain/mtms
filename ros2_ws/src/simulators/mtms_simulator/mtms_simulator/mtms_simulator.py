import time

import rclpy
from rclpy.node import Node
from rclpy.qos import (
    QoSProfile,
    DurabilityPolicy,
    HistoryPolicy,
    ReliabilityPolicy,
)
from rclpy.duration import Duration
from std_msgs.msg import String

from mtms_device_interfaces.msg import (
    SystemState,
    DeviceState,
    SessionState,
    SystemError,
    StartupError,
    ChannelState,
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
    Discharge,
    EventTrigger,
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
            StopSession, "/mtmt/stop_session", self.stop_session_handler
        )

        # Subscribers
        self.charge_subscriber = self.create_subscription(
            Charge, "/event/send/charge", self.charge_handler, 10
        )
        self.discharge_subscriber = self.create_subscription(
            Discharge, "/event/send/discharge", self.discharge_handler, 10
        )
        self.event_trigger_subscriber = self.create_subscription(
            EventTrigger, "/event/send/event_trigger", self.event_trigger_handler, 10
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
        self.system_state = SystemState()
        self.allow_stimulation = False

        self.create_timer(
            MTMSSimulator.SYSTEM_STATE_PUBLISHING_INTERVAL_MS / 1000,
            self.system_state_callback,
        )

    def allow_stimulation_handler(self, request, response):
        self.allow_stimulation = True
        response.success = True
        return response

    def send_settings_handler(self, request, response):
        pass

    def start_device_handler(self, request, response):
        # TODO: Check if STARTUP phase required
        self.system_state.device_state = DeviceState.OPERATIONAL
        response.success = True
        return response

    def stop_device_handler(self, request, response):
        self.system_state.device_state = DeviceState.SHUTDOWN
        response.success = True
        return response

    def start_session_handler(self, request, response):
        # TODO: Check if STARTING phase required
        self.system_state.session_state = SessionState.STARTED
        response.success = True
        return response

    def stop_session_handler(self, request, response):
        # TODO: Check if STOPPING phase required
        self.system_state.session_state = SessionState.STOPPED
        response.success = True
        return response

    def charge_handler(self, message):
        pass

    def discharge_handler(self, message):
        pass

    def event_trigger_handler(self, message):
        pass

    def pulse_handler(self, message):
        pass

    def trigger_out_handler(self, trigger_out):
        pass

    def system_state_callback(self):
        msg = self.system_state
        msg.time = time.time()  # in seconds, NOTE: MIGHT BE WRONG UNITS

        self.system_state_publisher.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    mtms_simulator = MTMSSimulator()
    rclpy.spin(mtms_simulator)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
