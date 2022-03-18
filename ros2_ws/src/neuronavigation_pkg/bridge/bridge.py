#!/usr/bin/env python3

from threading import Thread
from invesalius.pubsub import pub as Publisher

import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile

from geometry_msgs.msg import Point
from shape_msgs.msg import Mesh, MeshTriangle
from neuronavigation_interfaces.msg import PoseUsingEulerAngles

from std_msgs.msg import String

class NeuronavigationNode(Node):
    def __init__(self):
        super().__init__("neuronavigation")
        # Persist the latest sample.
        qos = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )
        self._coil_pose_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/coil_pose", 10)
        self._coil_mesh_publisher = self.create_publisher(Mesh, "neuronavigation/coil_mesh", qos)
        self._focus_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/focus", qos)

        self._efield_subscription = self.create_subscription(
            String,
            'efield',
            self.efield_listener_callback,
            10)
        self._efield_subscription  # prevent unused variable warning

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
        if orientation == [None, None, None]:
            orientation = [0.0, 0.0, 0.0]

        msg = PoseUsingEulerAngles()

        msg.position.x, msg.position.y, msg.position.z = position
        msg.orientation.alpha, msg.orientation.beta, msg.orientation.gamma = orientation

        self.get_logger().info("Publishing to the topic /neuronavigation/focus")
        self._focus_publisher.publish(msg)

    def update_coil_pose(self, position, orientation):
        msg = PoseUsingEulerAngles()
        if orientation == [None, None, None]:
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


def main():
    connection = Connection()
    connection.start()

    from invesalius3 import app
    app.main(connection=connection)


if __file__ == 'main':
    main()