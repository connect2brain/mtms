import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import PlannerState, Target
from mtms_interfaces.srv import RemoveTarget

from .state_node import StateNode


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


class RemoveTargetNode(StateNode):

    def __init__(self):
        super().__init__('remove_target')
        self.create_service(RemoveTarget, '/planner/remove_target', self.remove_target_callback)

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


def main():
    rclpy.init()

    remove_target_node = RemoveTargetNode()

    rclpy.spin(remove_target_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
