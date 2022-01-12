#!/usr/bin/env python3

from threading import Thread

import rclpy
from rclpy.node import Node
from neuronavigation_interfaces.msg import PoseUsingEulerAngles

class NeuronavigationNode(Node):
    def __init__(self):
        super().__init__("neuronavigation")

        self._coil_pose_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/coil_pose", 10)

    def update_coil_pose(self, coil_pose):
        msg = PoseUsingEulerAngles()

        msg.position.x, msg.position.y, msg.position.z = coil_pose[:3]
        msg.orientation.alpha, msg.orientation.beta, msg.orientation.gamma = coil_pose[3:]

        self.get_logger().info("Publishing to the topic /neuronavigation/coil_pose")
        self._coil_pose_publisher.publish(msg)


class Connection(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.daemon = True

        rclpy.init(args=None)
        self.node = NeuronavigationNode()

    def run(self):
        rclpy.spin(self.node)
        rclpy.shutdown()

    def update_coil_pose(self, coil_pose):
        self.node.update_coil_pose(coil_pose)


def main():
    connection = Connection()
    connection.start()

    from invesalius3 import app
    app.main(connection=connection)


if __file__ == 'main':
    main()