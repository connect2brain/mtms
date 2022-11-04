import os

import serial
import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from std_msgs.msg import Bool

class PedalListenerNode(Node):

    RECONNECT_PERIOD_IN_SECONDS = 1.0
    READER_PERIOD_IN_SECONDS = 0.02

    def __init__(self):
        super().__init__('pedal_listener')

        self._port = os.getenv("PEDAL_PORT")
        self._baud_rate = int(os.getenv("PEDAL_BAUD_RATE"))

        # Persist the latest sample.
        qos = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )

        self._pedal_connected_publisher = self.create_publisher(
            Bool,
            "/pedal/connected",
            qos
        )
        self._pedal_pressed_publisher = self.create_publisher(
            Bool,
            "/pedal/pressed",
            qos
        )

        self._serial = None
        self._reconnect_timer = self.create_timer(self.RECONNECT_PERIOD_IN_SECONDS, self._reconnect)
        self._reader_timer = self.create_timer(self.READER_PERIOD_IN_SECONDS, self._reader)

    def _handle_pedal_pressed(self):
        msg = Bool()
        msg.data = True
        self._pedal_pressed_publisher.publish(msg)
        self.get_logger().info("Pedal pressed.")

    def _handle_pedal_released(self):
        msg = Bool()
        msg.data = False
        self._pedal_pressed_publisher.publish(msg)
        self.get_logger().info("Pedal released.")

    def _handle_disconnected(self):
        self._serial = None

        msg = Bool()
        msg.data = False
        self._pedal_connected_publisher.publish(msg)

        self.get_logger().info("Pedal disconnected.")

    def _handle_connected(self):
        msg = Bool()
        msg.data = True
        self._pedal_connected_publisher.publish(msg)

        self.get_logger().info("Pedal connected.")

    def _reader(self):
        try:
            if self._serial is None:
                return

            value = self._serial.read()

            if len(value) == 0:
                return

            if value[0] == 0x00:
                self._handle_pedal_pressed()
            elif value[0] == 0x01:
                self._handle_pedal_released()
            else:
                self.get_logger().error("Unknown message received from the pedal.")

        except serial.serialutil.SerialException:
            self._handle_disconnected()

    def _reconnect(self):
        if self._serial is not None:
            return

        try:
            self.get_logger().info("Connecting to pedal in port {} using baud rate {}.".format(self._port, self._baud_rate))
            self._serial = serial.Serial(self._port, self._baud_rate)
            self._handle_connected()

        except serial.serialutil.SerialException:
            self._handle_disconnected()


def main():
    rclpy.init()
    pedal_listener_node = PedalListenerNode()
    rclpy.spin(pedal_listener_node)
    rclpy.shutdown()


if __name__ == '__main__':
    main()
