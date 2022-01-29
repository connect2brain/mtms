import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from geometry_msgs.msg import Point
from mtms_interfaces.msg import PlannerState, Target
from std_msgs.msg import Bool, Float64, String

from mtms_interfaces.srv import ToggleSelect


class ToggleSelectNode(Node):

    def __init__(self):
        super().__init__('toggle_select')
        self.create_service(ToggleSelect, '/planner/toggle_select', self.toggle_select_callback)

        # Persist the latest sample.
        qos = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )

        self._state_publisher = self.create_publisher(
            PlannerState,
            "/planner/state",
            qos
        )
        self._state_subscriber = self.create_subscription(
            PlannerState,
            '/planner/state',
            self.state_updated,
            10
        )
        self._state = None

    def state_updated(self, msg):
        self.get_logger().info('Planner state updated')
        self._state = msg

    def toggle_select_callback(self, request, response):
        self.get_logger().info('Incoming request')

        state = self._state
        if state is None:
            response.success = False
            return response

        for target in state.targets:
            if target.name == request.name:
                target.selected = not target.selected

        self._state_publisher.publish(state)

        response.success = True
        return response


def main():
    rclpy.init()

    toggle_select_node = ToggleSelectNode()

    rclpy.spin(toggle_select_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
