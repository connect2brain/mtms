import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import PlannerState, Pulse
from rclpy.qos import QoSProfile, DurabilityPolicy, HistoryPolicy, ReliabilityPolicy


class MasterStateNode(Node):

    def __init__(self):
        super().__init__('master_state')

        # Persist the latest sample.
        publish_qos = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            reliability=ReliabilityPolicy.RELIABLE,
        )

        subscribe_qos = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.VOLATILE,
            history=HistoryPolicy.KEEP_LAST,
            reliability=ReliabilityPolicy.RELIABLE,
        )

        self._state_publisher = self.create_publisher(
            PlannerState,
            "/planner/state",
            publish_qos
        )
        self._state_subscriber = self.create_subscription(
            PlannerState,
            '/planner/inner/state',
            self.state_updated,
            qos_profile=subscribe_qos
        )
        self._state = PlannerState()

    def state_updated(self, msg):
        self._state = msg
        self._state_publisher.publish(msg)
        self.get_logger().info(
            f'Updated state, nof targets: {len(self._state.targets)}, '
            f'nof sequences: {len(self._state.pulse_sequences)}')


def main():
    rclpy.init()
    get_state_node = MasterStateNode()
    rclpy.spin(get_state_node)
    rclpy.shutdown()


if __name__ == '__main__':
    main()
