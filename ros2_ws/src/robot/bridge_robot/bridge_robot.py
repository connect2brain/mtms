#!/usr/bin/env python3

import ctypes
import platform
import sys
from threading import Thread

import rclpy
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, ReliabilityPolicy, QoSProfile
from rcl_interfaces.msg import ParameterDescriptor, ParameterType

from geometry_msgs.msg import Point, Pose
from std_msgs.msg import Bool, MultiArrayDimension

from neuronavigation_interfaces.msg import EulerAngles, PoseUsingEulerAngles, MarkersVisibilities, PoseArray
from robot_interfaces.msg import ConnectToRobot, SetTarget, TrackerFiducials, MatrixTrackerToRobot, Displacement, Objective


from tms_robot_control import main_loop
import robot.transformations as tr
import numpy as np
import time
from launch.substitutions import LaunchConfiguration


# TODO: Divide this large class into several nodes.
#
class RobotNode(Node):
    # HACK: Needs to match the corresponding value in stimulation allower ROS node.
    COIL_AT_TARGET_DEADLINE_S = 0.6
    def __init__(self):
        super().__init__("robot")

        ## ROS parameters

        # Create publishers, subscribers, and services
        qos_persist_latest = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )
        callback_group = ReentrantCallbackGroup()

        self._poses_subscription = self.create_subscription(PoseArray, "neuronavigation/poses",
                                                            self.update_poses_callback, qos_persist_latest, callback_group=callback_group)
        self._coil_at_target_subscription = self.create_subscription(Bool, "neuronavigation/coil_at_target",
                                                                     self.update_coil_at_target_callback, qos_persist_latest, callback_group=callback_group)
        self._connect_to_robot_subscription = self.create_subscription(ConnectToRobot, "neuronavigation/connect_to_robot",
                                                                       self.update_connect_to_robot_callback, qos_persist_latest, callback_group=callback_group)
        self._set_target_subscription = self.create_subscription(SetTarget, "neuronavigation/set_target",
                                                                  self.update_set_target_callback, qos_persist_latest, callback_group=callback_group)
        self._unset_target_subscription = self.create_subscription(Bool, "neuronavigation/unset_target",
                                                                  self.update_unset_target_callback, qos_persist_latest, callback_group=callback_group)
        self._set_tracker_fiducials_subscription = self.create_subscription(TrackerFiducials, "neuronavigation/set_tracker_fiducials",
                                                                  self.update_set_tracker_fiducials_callback, qos_persist_latest, callback_group=callback_group)
        self._collect_robot_pose_subscription = self.create_subscription(Bool, "neuronavigation/collect_robot_pose",
                                                                  self.update_collect_robot_pose_callback, qos_persist_latest, callback_group=callback_group)
        self._reset_robot_transformation_matrix_subscription = self.create_subscription(Bool, "neuronavigation/reset_robot_transformation_matrix",
                                                                  self.update_reset_robot_transformation_matrix_callback, qos_persist_latest, callback_group=callback_group)
        self._estimate_robot_transformation_matrix_subscription = self.create_subscription(Bool, "neuronavigation/estimate_robot_transformation_matrix",
                                                                  self.update_estimate_robot_transformation_matrix_callback, qos_persist_latest, callback_group=callback_group)
        self._set_robot_transformation_matrix_subscription = self.create_subscription(MatrixTrackerToRobot, "neuronavigation/set_robot_transformation_matrix",
                                                                  self.set_robot_transformation_matrix_callback, qos_persist_latest, callback_group=callback_group)
        self._update_displacement_to_target_subscription = self.create_subscription(Displacement, "neuronavigation/update_displacement_to_target",
                                                                  self.update_displacement_to_target_callback, qos_persist_latest, callback_group=callback_group)
        self._set_objective_subscription = self.create_subscription(Objective, "neuronavigation/set_objective",
                                                                  self.update_set_objective_callback, qos_persist_latest, callback_group=callback_group)

        self._update_robot_status_publisher = self.create_publisher(Bool, "/robot/update_robot_status",
                                                                 qos_persist_latest, callback_group=callback_group)
        self._robot_connection_status_publisher = self.create_publisher(Bool, "/robot/robot_connection_status",
                                                                 qos_persist_latest, callback_group=callback_group)
        self._robot_pose_collected_publisher = self.create_publisher(Bool, "/robot/robot_pose_collected",
                                                                 qos_persist_latest, callback_group=callback_group)
        self._set_objective_publisher = self.create_publisher(Objective, "/robot/set_objective",
                                                                 qos_persist_latest, callback_group=callback_group)
        self._close_robot_dialog_publisher = self.create_publisher(Bool, "/robot/close_robot_dialog",
                                                                 qos_persist_latest, callback_group=callback_group)
        self._update_robot_transformation_matrix_publisher = self.create_publisher(MatrixTrackerToRobot, "/robot/update_robot_transformation_matrix",
                                                                                qos_persist_latest, callback_group=callback_group)

    def set_callback__connect_to_robot(self, callback):
        self._connect_to_robot = callback

    def update_connect_to_robot_callback(self, msg):
        self._connect_to_robot(msg.robot_ip)
        self.get_logger().info('I heard connect_to_robot: "%s"' % msg)

    def set_callback__coil_at_target(self, callback):
        self._coil_at_target = callback

    def update_coil_at_target_callback(self, msg):
        self._coil_at_target(msg.data)
        #self.get_logger().info('I heard coil_at_target: "%s"' % msg)

    def set_callback__set_target(self, callback):
        self._set_target = callback

    def update_set_target_callback(self, msg):
        self._set_target(msg.target.data)
        self.get_logger().info('I heard set_target: "%s"' % msg)

    def set_callback__unset_target(self, callback):
        self._unset_target = callback

    def update_unset_target_callback(self, data):
        self.get_logger().info('I heard unset_target')

    def set_callback__update_poses(self, callback):
        self._update_poses = callback

    def update_poses_callback(self, msg):
        poses = []
        poses_probe = msg.poses[0]
        poses_head = msg.poses[1]
        poses_coil = msg.poses[2]
        for marker_pose in [poses_probe, poses_head, poses_coil]:
            x, y, z = marker_pose.position.x, marker_pose.position.y, marker_pose.position.z
            quaternion = [marker_pose.orientation.x, marker_pose.orientation.y, marker_pose.orientation.z, marker_pose.orientation.w]
            ai, aj, ak = np.rad2deg(tr.euler_from_quaternion(quaternion))
            poses.append([x, y, z, ai, aj, ak])
        visibilities = [msg.visibilities.probe, msg.visibilities.head, msg.visibilities.coil]
        self._update_poses(poses, visibilities)
        #self.get_logger().info('I heard update_poses: "%s"' % msg)

    def set_callback__set_tracker_fiducials(self, callback):
        self._set_tracker_fiducials = callback

    def update_set_tracker_fiducials_callback(self, msg):
        self._set_tracker_fiducials(msg.fiducial_left.data, msg.fiducial_right.data, msg.fiducial_nasion.data)
        self.get_logger().info('I heard set_tracker_fiducials: "%s"' % msg)

    def set_callback__collect_robot_pose(self, callback):
        self._collect_robot_pose = callback

    def update_collect_robot_pose_callback(self, msg):
        self._collect_robot_pose(msg.data)
        self.get_logger().info('I heard neuronavigation/collect_robot_pose: "%s"' % msg)

    def set_callback__reset_robot_transformation_matrix(self, callback):
        self._reset_robot_transformation_matrix = callback

    def update_reset_robot_transformation_matrix_callback(self, msg):
        self._reset_robot_transformation_matrix(msg.data)
        self.get_logger().info('I heard reset_robot_transformation_matrix: "%s"' % msg)

    def set_callback__estimate_robot_transformation_matrix(self, callback):
        self._estimate_robot_transformation_matrix = callback

    def update_estimate_robot_transformation_matrix_callback(self, msg):
        self._estimate_robot_transformation_matrix(msg.data)
        self.get_logger().info('I heard estimate_robot_transformation_matrix: "%s"' % msg)

    def set_callback__set_robot_transformation_matrix(self, callback):
        self._set_robot_transformation_matrix = callback

    def set_robot_transformation_matrix_callback(self, msg):
        self._set_robot_transformation_matrix(msg.robotmatrix.data)
        self.get_logger().info('I heard neuronavigation/set_robot_transformation_matrix: "%s"' % msg)

    def set_callback__update_displacement_to_target(self, callback):
        self._update_displacement_to_target = callback

    def update_displacement_to_target_callback(self, msg):
        self._update_displacement_to_target(msg.displacement)
        #self.get_logger().info('I heard update_displacement_to_target: "%s"' % msg)

    def set_callback__set_objective(self, callback):
        self._set_objective = callback

    def update_set_objective_callback(self, msg):
        self._set_objective(msg.objective)
        self.get_logger().info('I heard set_objective: "%s"' % msg)

    def update_robot_status(self, success):
        msg = Bool()
        msg.data = success
        self.get_logger().info("Publishing value {} to the topic /robot/update_robot_status".format(msg))
        self._update_robot_status_publisher.publish(msg)

    def robot_connection_status(self, success):
        msg = Bool()
        msg.data = success
        self.get_logger().info("Publishing value {} to the topic /robot/robot_connection_status".format(msg))
        self._robot_connection_status_publisher.publish(msg)

    def robot_pose_collected(self, success):
        msg = Bool()
        msg.data = success
        self.get_logger().info("Publishing value {} to the topic /robot/robot_pose_collected".format(msg))
        self._robot_pose_collected_publisher.publish(msg)

    def set_objective(self, objective):
        msg = Objective()
        msg.objective = objective
        self.get_logger().info("Publishing value {} to the topic /robot/set_objective".format(msg))
        self._set_objective_publisher.publish(msg)

    def close_robot_dialog(self, success):
        msg = Bool()
        msg.data = success
        self.get_logger().info("Publishing value {} to the topic /robot/close_robot_dialog".format(msg))
        self._close_robot_dialog_publisher.publish(msg)

    def update_robot_transformation_matrix(self, matrix):
        msg = MatrixTrackerToRobot()
        msg.robotmatrix.data = matrix
        self.get_logger().info("Publishing value {} to the topic /robot/update_robot_transformation_matrix".format(msg))
        self._update_robot_transformation_matrix_publisher.publish(msg)



class Connection(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.daemon = True

        rclpy.init(args=None)
        self.node = RobotNode()

        self.executor = rclpy.executors.MultiThreadedExecutor()
        self.executor.add_node(self.node)

    def run(self):
        self.executor.spin()
        rclpy.shutdown()

    def set_callback__connect_to_robot(self, callback):
        self.node.set_callback__connect_to_robot(callback)

    def set_callback__coil_at_target(self, callback):
        self.node.set_callback__coil_at_target(callback)

    def set_callback__set_target(self, callback):
        self.node.set_callback__set_target(callback)

    def set_callback__unset_target(self, callback):
        self.node.set_callback__unset_target(callback)

    def set_callback__update_poses(self, callback):
        self.node.set_callback__update_poses(callback)

    def set_callback__set_tracker_fiducials(self, callback):
        self.node.set_callback__set_tracker_fiducials(callback)

    def set_callback__collect_robot_pose(self, callback):
        self.node.set_callback__collect_robot_pose(callback)

    def set_callback__reset_robot_transformation_matrix(self, callback):
        self.node.set_callback__reset_robot_transformation_matrix(callback)

    def set_callback__estimate_robot_transformation_matrix(self, callback):
        self.node.set_callback__estimate_robot_transformation_matrix(callback)

    def set_callback__set_robot_transformation_matrix(self, callback):
        self.node.set_callback__set_robot_transformation_matrix(callback)

    def set_callback__update_displacement_to_target(self, callback):
        self.node.set_callback__update_displacement_to_target(callback)

    def set_callback__set_objective(self, callback):
        self.node.set_callback__set_objective(callback)

    def update_robot_status(self, success):
        self.node.update_robot_status(
            success=success,
        )

    def robot_connection_status(self, success):
        self.node.robot_connection_status(
            success=success,
        )

    def robot_pose_collected(self, success):
        self.node.robot_pose_collected(
            success=success,
        )

    def set_objective(self, objective):
        self.node.set_objective(
            objective=objective,
        )

    def close_robot_dialog(self, success):
        self.node.close_robot_dialog(
            success=success,
        )

    def update_robot_transformation_matrix(self, matrix):
        self.node.update_robot_transformation_matrix(
            matrix=matrix,
        )



class RosLoggerWrapper:
    def __init__(self, node):
        self.node = node

    def write(self, message):
        if message.rstrip() != "":
            # Redirect to ROS2's logging.
            self.node.get_logger().info(message.rstrip())

    def flush(self):
        pass


def main():
    connection = Connection()
    connection.start()

    # Override stdout to redirect print statements to ROS2 logging.
    sys.stdout = RosLoggerWrapper(connection.node)

    if platform.system() != 'Windows':
        # XInitThreads call is needed for multithreading in InVesalius to not crash when running in Docker.
        x11 = ctypes.cdll.LoadLibrary('libX11.so')
        x11.XInitThreads()

    main_loop.main(connection=connection)


if __file__ == 'main':
    main()
