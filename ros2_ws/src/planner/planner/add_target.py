import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import PlannerState, Target
from std_msgs.msg import Bool, Float64, String
from mtms_interfaces.srv import AddTarget

from .state_node import StateNode

class AddTargetNode(StateNode):

    def __init__(self):
        super().__init__('add_target')
        self.create_service(AddTarget, '/planner/add_target', self.add_target_callback)

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

    add_target_node = AddTargetNode()

    rclpy.spin(add_target_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
