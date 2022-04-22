#!/usr/bin/env python3

import ctypes
from threading import Thread

import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from geometry_msgs.msg import Point
from shape_msgs.msg import Mesh, MeshTriangle
from std_msgs.msg import Bool

from neuronavigation_interfaces.msg import PoseUsingEulerAngles
from neuronavigation_interfaces.srv import Efield
from mtms_interfaces.msg import PlannerState

from invesalius3 import app


class NeuronavigationNode(Node):
    # The colors have been picked from mTMS software prototype created in Adobe XD.
    #
    # XXX: The colors match those that are defined in _colors.scss in front-end. It would be
    #      better if they were defined in a single place.
    #
    _COLOR_TARGET = (43, 197, 255)  # hex: #2BC5FF, $target-color
    _COLOR_NON_TARGET = (230, 98, 48)  # hex: #E66230, $non-target-color
    _COLOR_SELECTED = (112, 112, 112)  # hex: #707070, $darker-gray

    def __init__(self):
        super().__init__("neuronavigation")

        qos_persist_latest = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )
        self._coil_pose_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/coil_pose", 10)
        self._coil_at_target_publisher = self.create_publisher(Bool, "neuronavigation/coil_at_target", 10)

        self._coil_mesh_publisher = self.create_publisher(Mesh, "neuronavigation/coil_mesh", qos_persist_latest)
        self._focus_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/focus", qos_persist_latest)
        self._planner_state_subscription = self.create_subscription(PlannerState, "planner/state", self.planner_state_callback, qos_persist_latest)

        self.cli = self.create_client(Efield, 'efield')
        while not self.cli.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('efield service not available, waiting again...')
        self.req = Efield.Request()

    def set_callback__set_markers(self, callback):
        self._set_markers = callback

    def planner_state_callback(self, msg):
        markers = []
        for id, target in enumerate(msg.targets):
            position = [target.pose.position.x, target.pose.position.y, target.pose.position.z]
            direction = [target.pose.orientation.alpha, target.pose.orientation.beta, target.pose.orientation.gamma]

            if target.target:
                color = self._COLOR_TARGET

            elif target.selected:
                color = self._COLOR_SELECTED

            else:
                color = self._COLOR_NON_TARGET

            marker_data = {
                'ball_id': id,
                'position': position,
                'direction': direction,
                'target': target.target,
                'size': 3,
                'colour': [c / 255.0 for c in color],
            }
            markers.append(marker_data)

        self._set_markers(markers)
        self.get_logger().info('I heard planner state')

    def efield_listener_callback(self, msg):
        self.get_logger().info('I heard efield: "%s"' % msg.data)
        #Publisher.sendMessage('invesalius messages', arg=msg.data)

    def update_focus(self, position, orientation):
        # TODO: The Euler angles cannot be None in the ROS message, hence the lines
        #   below. Most likely the correct change to be able to remove this is to
        #   decouple updates of focus that include orientation (during navigation)
        #   from the updates that do not include orientation (outside navigation,
        #   using mouse) into two different messages. However, it should be considered
        #   if the mouse could be used to set a proper stimulation target, i.e., one
        #   vector for position and another for orientation - in that case, there'd
        #   be no more the possibility of passing None values here
        #   and this check could be removed.
        #
        if all(x is None for x in orientation):
            orientation = [0.0, 0.0, 0.0]

        msg = PoseUsingEulerAngles()

        msg.position.x, msg.position.y, msg.position.z = position
        msg.orientation.alpha, msg.orientation.beta, msg.orientation.gamma = orientation

        self.get_logger().info("Publishing to the topic /neuronavigation/focus")
        self._focus_publisher.publish(msg)

    def update_coil_at_target(self, state):
        msg = Bool()
        msg.data = state
        self.get_logger().info("Publishing to the topic /neuronavigation/coil_at_target")
        self._coil_at_target_publisher.publish(msg)

    def update_coil_pose(self, position, orientation):
        msg = PoseUsingEulerAngles()
        if all(x is None for x in orientation):
            orientation = [0.0, 0.0, 0.0]

        msg.position.x, msg.position.y, msg.position.z = position
        msg.orientation.alpha, msg.orientation.beta, msg.orientation.gamma = orientation

        self.get_logger().info("Publishing to the topic /neuronavigation/coil_pose")
        self._coil_pose_publisher.publish(msg)

    def update_coil_mesh(self, points, polygons):
        msg = Mesh()

        msg.vertices = [Point(x=point[0], y=point[1], z=point[2]) for point in points.astype(float)]
        msg.triangles = [MeshTriangle(vertex_indices=polygon) for polygon in polygons.astype(int)]

        self.get_logger().info("Publishing to the topic /neuronavigation/coil_mesh")
        self._coil_mesh_publisher.publish(msg)

    def update_efield(self, position, orientation):
        self.req.coordinate.position.x, self.req.coordinate.position.y, self.req.coordinate.position.z = position
        self.req.coordinate.orientation.alpha, self.req.coordinate.orientation.beta, self.req.coordinate.orientation.gamma = orientation

        self.future = self.cli.call_async(self.req)
        while self.future.done() is False:
            pass
        try:
            response = self.future.result()
            self.get_logger().info("Responding to the service request /neuronavigation/efield")
            return response.efield_data
        except Exception as e:
            self.get_logger().info(
                'Service call failed %r' % (e,))
            return None


class Connection(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.daemon = True

        rclpy.init(args=None)
        self.node = NeuronavigationNode()

    def run(self):
        rclpy.spin(self.node)
        rclpy.shutdown()

    def update_focus(self, position, orientation):
        self.node.update_focus(
            position=position,
            orientation=orientation,
        )

    def update_coil_at_target(self, state):
        self.node.update_coil_at_target(
            state=state,
        )

    def update_coil_pose(self, position, orientation):
        self.node.update_coil_pose(
            position=position,
            orientation=orientation,
        )

    def update_coil_mesh(self, points, polygons):
        self.node.update_coil_mesh(
            points=points,
            polygons=polygons,
        )

    def update_efield(self, position, orientation):
        return self.node.update_efield(
                    position=position,
                    orientation=orientation,
                )

    def set_callback__set_markers(self, callback):
        self.node.set_callback__set_markers(callback)


def main():
    connection = Connection()
    connection.start()

    x11 = ctypes.cdll.LoadLibrary('libX11.so')
    x11.XInitThreads()

    app.main(connection=connection)


if __file__ == 'main':
    main()