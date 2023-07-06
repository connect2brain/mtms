#!/usr/bin/env python3

import ctypes
import platform
from threading import Thread

import rclpy
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile
from rcl_interfaces.msg import ParameterDescriptor, ParameterType

from geometry_msgs.msg import Point
from shape_msgs.msg import Mesh, MeshTriangle
from std_msgs.msg import Bool

from neuronavigation_interfaces.msg import EulerAngles, PoseUsingEulerAngles, OptitrackPoses, ElectricField
from neuronavigation_interfaces.srv import Efield, OpenOrientationDialog, InitializeEfield, SetCoil, EfieldNorm
from ui_interfaces.msg import PlannerState
from ui_interfaces.srv import SetTargetOrientation

from invesalius3 import app
import time
from launch.substitutions import LaunchConfiguration

from .neuronavigation_pedal_bridge import NeuronavigationPedalBridge


# TODO: Divide this large class into several nodes.
#
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
        callback_group = ReentrantCallbackGroup()

        self._coil_pose_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/coil_pose", 10, callback_group=callback_group)
        self._coil_at_target_publisher = self.create_publisher(Bool, "neuronavigation/coil_at_target", 10, callback_group=callback_group)

        self._coil_mesh_publisher = self.create_publisher(Mesh, "neuronavigation/coil_mesh", qos_persist_latest, callback_group=callback_group)
        self._focus_publisher = self.create_publisher(PoseUsingEulerAngles, "neuronavigation/focus", qos_persist_latest, callback_group=callback_group)
        self._planner_state_subscription = self.create_subscription(PlannerState, "planner/state",
                                                                    self.planner_state_callback, qos_persist_latest, callback_group=callback_group)
        self._optitrack_state_subscription = self.create_subscription(OptitrackPoses, "/neuronavigation/optitrack_poses",
                                                                      self.optitrack_listener_callback, 1, callback_group=callback_group)

        self._open_orientation_dialog_service = self.create_service(OpenOrientationDialog,
                                                                    "neuronavigation/open_orientation_dialog",
                                                                    self.open_orientation_dialog_callback, callback_group=callback_group)

        self._update_target_orientation_client = self.create_client(SetTargetOrientation,
                                                                    '/planner/set_target_orientation', callback_group=callback_group)
        # descriptor = ParameterDescriptor(
        #     name='EField',
        #     type=ParameterType.PARAMETER_BOOL,
        # )
        # self.declare_parameter('Activate_EField', descriptor=descriptor)
        # self.Activate_EField = self.get_parameter('Activate_EField').value
        self.Activate_EField = True
        if self.Activate_EField:
            self.client_init_efield = self.create_client(InitializeEfield, '/efield/initialize', callback_group=callback_group)
            while not self.client_init_efield.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('efield service /efield/init not available, waiting...')
            self.get_logger().info('efield enorm')

            self.client_get_efield_norm = self.create_client(EfieldNorm, '/efield/get_norm', callback_group=callback_group)
            while not self.client_get_efield_norm.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('efield service /efield/getnorm not available, waiting...')

            self.client_get_efield_vector = self.create_client(Efield, '/efield/get_efieldvector', callback_group=callback_group)
            while not self.client_get_efield_vector.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('efield service /efield/get_efieldvector not available, waiting...')

            self.client_set_coil = self.create_client(SetCoil, '/efield/set_coil', callback_group= callback_group)
            while not self.client_set_coil.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('efield service /efield/set_coil not available, waiting...')
            self.get_logger().info('efield set coil')

    def set_callback__set_markers(self, callback):
        self._set_markers = callback

    def set_callback__open_orientation_dialog(self, callback):
        self._open_orientation_dialog = callback

    def open_orientation_dialog_callback(self, request, response):
        target_id = request.target_id
        self.get_logger().info(f'Received open orientation dialog with target {target_id}')
        self._open_orientation_dialog(target_id)

        response.success = True
        return response

    def planner_state_callback(self, msg):
        if not hasattr(self, '_set_markers'):
            self.get_logger().info('set markers not set yet')
            return

        self.get_logger().info(f'Updating planner state')
        markers = []
        for id, target in enumerate(msg.targets):
            position = [target.pose.position.x, target.pose.position.y, target.pose.position.z]
            orientation = [target.pose.orientation.alpha, target.pose.orientation.beta, target.pose.orientation.gamma]

            if target.target:
                color = self._COLOR_TARGET

            elif target.selected:
                color = self._COLOR_SELECTED

            else:
                color = self._COLOR_NON_TARGET

            marker_data = {
                'ball_id': id,
                'position': position,
                'orientation': orientation,
                'target': target.target,
                'size': 3,
                'colour': [c / 255.0 for c in color],
                'arrow_flag': not all(d == 0.0 for d in orientation)
            }
            markers.append(marker_data)

        self._set_markers(markers)
        self.get_logger().info('I heard planner state')

    def optitrack_listener_callback(self, msg):
        self.get_logger().info('I heard optitrack: "%s"' % msg.probe)
        # Simulate a very slow consumer
        time.sleep(0.1)
    def efield_listener_callback(self, msg):
        self.get_logger().info('I heard efield: "%s"' % msg.data)
        # Publisher.sendMessage('invesalius messages', arg=msg.data)

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

    def initialize_efield(self, cortex_model_path, mesh_models_paths, coil_model_path, conductivities_inside, conductivities_outside):
        request = InitializeEfield.Request()
        request.cortex_model_path= cortex_model_path
        request.mesh_models_paths = mesh_models_paths
        request.coil_model_path = coil_model_path
        request.conductivities_inside = conductivities_inside
        request.conductivities_outside = conductivities_outside
        future = self.client_init_efield.call_async(request)
        while future.done() is False:
            pass
        try:
            response= future.result()
            self.get_logger().info("Responding to the service request /neuronavigation/efield/init")
            return response.success
        except Exception as e:
            self.get_logger().info('Service call failed %r' % (e,))
            return None

    def set_coil(self, coil_model_path):
        request = SetCoil.Request()
        request.coil_model_path=coil_model_path
        future = self.client_set_coil.call_async(request)
        while future.done() is False:
            pass
        try:
            response= future.result()
            self.get_logger().info("Responding to the service request /neuronavigation/efield/coil")
            return response.success
        except Exception as e:
            self.get_logger().info('Service call failed %r' % (e,))
            return None

    def update_efield(self, position, orientation, T_rot):
        request= EfieldNorm.Request()
        request.coordinate.position.x, request.coordinate.position.y, request.coordinate.position.z = position
        request.coordinate.orientation.alpha, request.coordinate.orientation.beta, request.coordinate.orientation.gamma = orientation
        request.transducer_rotation = T_rot

        future = self.client_get_efield_norm.call_async(request)

        while future.done() is False:
            pass
        try:
            response = future.result()
            self.get_logger().info("Responding to the service request /neuronavigation/efield")
            return response.efield_norm
        except Exception as e:
            self.get_logger().info('Service call failed %r' % (e,))
            return None

    def update_efield_vector(self,position, orientation, T_rot):
        request = Efield.Request()
        request.coordinate.position.x, request.coordinate.position.y, request.coordinate.position.z = position
        request.coordinate.orientation.alpha, request.coordinate.orientation.beta, request.coordinate.orientation.gamma = orientation
        request.transducer_rotation = T_rot

        future = self.client_get_efield_vector.call_async(request)

        while future.done() is False:
            pass
        try:
            response = future.result()
            self.get_logger().info("Responding to the service request /neuronavigation/efield_vector")
            return response.efield_data
        except Exception as e:
            self.get_logger().info('Service call failed %r' % (e,))
            return None

    def update_target_orientation(self, target_id, orientation):
        self.get_logger().info(f'updating target {target_id} orientation to {str(orientation)}')
        request = SetTargetOrientation.Request()

        euler_angles = EulerAngles()
        euler_angles.alpha = orientation[0]
        euler_angles.beta = orientation[1]
        euler_angles.gamma = orientation[2]

        request.orientation = euler_angles
        request.target_id = target_id

        self._update_target_orientation_client.call_async(request)


class Connection(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.daemon = True

        rclpy.init(args=None)
        self.node = NeuronavigationNode()
        self.neuronavigation_pedal_bridge = NeuronavigationPedalBridge()

        self.executor = rclpy.executors.MultiThreadedExecutor()
        self.executor.add_node(self.node)
        self.executor.add_node(self.neuronavigation_pedal_bridge)

    def run(self):
        self.executor.spin()
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

    def update_target_orientation(self, target_id, orientation):
        self.node.update_target_orientation(
            target_id=target_id,
            orientation=orientation
        )

    def update_coil_mesh(self, points, polygons):
        self.node.update_coil_mesh(
            points=points,
            polygons=polygons,
        )

    def update_efield(self, position, orientation, T_rot):
        return self.node.update_efield(
            position=position,
            orientation=orientation,
            T_rot=T_rot,
        )

    def update_efield_vector(self, position, orientation, T_rot):
        return self.node.update_efield_vector(
            position=position,
            orientation=orientation,
            T_rot=T_rot,
        )

    def initialize_efield(self, cortex_model_path, mesh_models_paths,  coil_model_path, conductivities_inside, conductivities_outside):
        return self.node.initialize_efield(
            cortex_model_path=cortex_model_path,
            mesh_models_paths=mesh_models_paths,
            coil_model_path=coil_model_path,
            conductivities_inside=conductivities_inside,
            conductivities_outside=conductivities_outside,
        )

    def set_coil(self, coil_model_path):
        return self.node.set_coil(
            coil_model_path=coil_model_path
        )

    def set_callback__set_markers(self, callback):
        self.node.set_callback__set_markers(callback)

    def set_callback__open_orientation_dialog(self, callback):
        self.node.set_callback__open_orientation_dialog(callback)

    def add_pedal_callback(self, name, callback, remove_when_released=False):
        self.neuronavigation_pedal_bridge.add_pedal_callback(
            name=name,
            callback=callback,
            remove_when_released=remove_when_released,
        )

    def remove_pedal_callback(self, name):
        self.neuronavigation_pedal_bridge.remove_pedal_callback(name=name)


def main():
    connection = Connection()
    connection.start()

    if platform.system() != 'Windows':
        # XInitThreads call is needed for multithreading in InVesalius to not crash when running in Docker.
        x11 = ctypes.cdll.LoadLibrary('libX11.so')
        x11.XInitThreads()

    app.main(connection=connection)


if __file__ == 'main':
    main()
