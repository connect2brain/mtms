import os

import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from std_msgs.msg import Bool
from mtms_device_interfaces.msg import SystemState, SessionState, DeviceState

from busylight.lights import Light
from busylight.lights.exceptions import NoLightsFound

class PedalListenerNode(Node):

    RECONNECT_PERIOD_IN_SECONDS = 1.0
    READER_PERIOD_IN_SECONDS = 0.02

    COLOR_DEVICE_STARTUP = (16, 16, 16)
    COLOR_DEVICE_OPERATIONAL = (64, 64, 64)
    COLOR_DEVICE_SHUTDOWN = (16, 16, 16)

    COLOR_SESSION_STARTED = (128, 0, 0)
    COLOR_SESSION_STOPPING = (128, 70, 0)

    REFRESH_PERIOD_IN_SECONDS = 25

    def __init__(self):
        super().__init__('busylight_manager')

        # Persist the latest sample.
        qos = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )

        # Create subscriber for system state.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1)

        # XXX: Kuando Omega busylight seems to turn itself off automatically after 30 seconds. To work around that, refresh the state periodically.
        self._reconnect_timer = self.create_timer(self.REFRESH_PERIOD_IN_SECONDS, self.refresh_light)

        self.session_state = None
        self.light = None

        self.current_color = None

    def refresh_light(self):
        if self.light is None:
            return

        if self.current_color is None:
            self.light.off()
        else:
            self.light.on(self.current_color)

    def handle_system_state(self, msg):
        session_state = msg.session_state.value
        device_state = msg.device_state.value

        state_changed = self.session_state != session_state or self.device_state != device_state

        self.session_state = session_state
        self.device_state = device_state

        # Only modify the light if either session or device state has changed.
        if not state_changed:
            return

        # Try to reconnect each time state changes in case the light has been disconnected and reconnected meanwhile.
        self.reconnect()

        # If no busylight is found, do not do anything.
        if self.light is None:
            return

        if self.device_state == DeviceState.OPERATIONAL:
            if self.session_state == SessionState.STARTING:
                pass

            elif self.session_state == SessionState.STARTED:
                self.get_logger().info("Device operational, session started.")
                self.current_color = self.COLOR_SESSION_STARTED

            elif self.session_state == SessionState.STOPPING:
                self.get_logger().info("Device operational, session stopping.")
                self.current_color = self.COLOR_SESSION_STOPPING

            elif self.session_state == SessionState.STOPPED:
                self.get_logger().info("Device operational, session stopped.")
                self.current_color = self.COLOR_DEVICE_OPERATIONAL

            else:
                self.get_logger().error("Unknown session state.")

        elif self.device_state == DeviceState.STARTUP:
            self.get_logger().info("Device starting up.")
            self.current_color = self.COLOR_DEVICE_STARTUP

        elif self.device_state == DeviceState.SHUTDOWN:
            self.get_logger().info("Device shutting down.")
            self.current_color = self.COLOR_DEVICE_SHUTDOWN

        elif self.device_state == DeviceState.NOT_OPERATIONAL:
            self.get_logger().info("Device not operational, turning off.")
            self.current_color = None

        else:
            self.get_logger().error("Unknown device state.")

        self.refresh_light()

    def reconnect(self):
        try:
            self.light = Light.first_light()
        except NoLightsFound as e:
            pass

def main():
    rclpy.init()
    busylight_manager_node = PedalListenerNode()
    rclpy.spin(busylight_manager_node)
    rclpy.shutdown()


if __name__ == '__main__':
    main()
