import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from geometry_msgs.msg import Point
from mtms_interfaces.msg import PlannerState, Target
from std_msgs.msg import Bool, Float64, String

from mtms_interfaces.srv import AddTarget


class Planner(Node):

    def __init__(self):
        super().__init__('planner')
        self.srv = self.create_service(AddTarget, '/planner/add_target', self.add_target_callback)

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

    def create_new_target(self, pose):
        target_idx = 0 if self._state is None else len(self._state.targets)

        target = Target()

        target.name = "Target-{}".format(target_idx)
        target.type = "Target"
        target.comment = ""
        target.selected = False
        target.target = False  # XXX: Misnomer
        target.pose = pose

        target.intensity = 100.0
        target.iti = 100.0

        return target

    def add_target_callback(self, request, response):
        self.get_logger().info('Incoming request')

        target = self.create_new_target(
            pose=request.target  # XXX: Misnomer
        )

        if self._state is None:
            msg = PlannerState()
            msg.targets = [
                target
            ]
        else:
            msg = self._state
            msg.targets.append(target)

        self._state_publisher.publish(msg)

        response.success = True
        return response


def main():
    rclpy.init()

    planner = Planner()

    rclpy.spin(planner)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
