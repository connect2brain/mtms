import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from geometry_msgs.msg import Point
from mtms_interfaces.msg import PlannerState, Target
from std_msgs.msg import Bool, Float64, String

from mtms_interfaces.srv import AddTarget, RemoveTarget, ToggleSelect


# TODO: Move this from here to some utils-like module
#
def drop_unique(l, cond):
    """Given a list and a condition, drop the element for which the condition is True.
    Modify the list in place. Return the index of the dropped element.

    If the condition is True for none of the elements or more than one element, raise an error.

    Parameters
    ----------
    l
        The list of interest.
    cond
        A function that takes an element of the list and return a boolean.
    """
    index = None
    for i, element in enumerate(l):
        if cond(element):
            assert index is None, "Condition is True for more than one element"
            index = i
    assert index is not None, "Condition is True for none of the elements"
    del l[index]

    return index


class Planner(Node):

    def __init__(self):
        super().__init__('planner')
        self.create_service(AddTarget, '/planner/add_target', self.add_target_callback)
        self.create_service(RemoveTarget, '/planner/remove_target', self.remove_target_callback)
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

    def remove_target_callback(self, request, response):
        self.get_logger().info('Incoming request')

        state = self._state
        if state is None:
            response.success = False
            return response

        idx = drop_unique(state.targets, lambda target: target.name == request.name)

        self._state_publisher.publish(state)

        response.success = True
        return response

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

    planner = Planner()

    rclpy.spin(planner)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
