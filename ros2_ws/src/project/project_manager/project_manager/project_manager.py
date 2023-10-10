import os

from project_interfaces.srv import ListProjects, SetActiveProject
from std_msgs.msg import String

import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile


class ProjectManagerNode(Node):
    PROJECTS_ROOT = '/app/projects'

    def __init__(self):
        super().__init__('project_manager_node')

        self.logger = self.get_logger()

        # Create service for listing projects.

        self.service = self.create_service(
            ListProjects,
            '/projects/list',
            self.list_projects_callback,
        )

        # Create service for setting active project.

        self.service = self.create_service(
            SetActiveProject,
            '/projects/active/set',
            self.set_active_project_callback,
        )

        # Create publisher for publishing active project.
        qos_persist_latest = QoSProfile(
            depth=1,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
        )
        self.active_project_publisher = self.create_publisher(String, "/projects/active", qos_persist_latest)

        # Internal state to store the active project.
        self.active_project = ""

    def list_projects_callback(self, request, response):
        try:
            # List directories in PROJECTS_ROOT
            response.projects = [d for d in os.listdir(self.PROJECTS_ROOT) if os.path.isdir(os.path.join(self.PROJECTS_ROOT, d))]

            self.logger.info("Projects successfully listed.")
            response.success = True

        except Exception as e:
            self.logger.error("Error listing projects: {}.".format(e))
            response.success = False

        return response

    def set_active_project_callback(self, request, response):
        project = request.project

        # Check if the specified project exists
        if project in [d for d in os.listdir(self.PROJECTS_ROOT) if os.path.isdir(os.path.join(self.PROJECTS_ROOT, d))]:
            self.active_project = project

            # Publish the active project
            msg = String()
            msg.data = project
            self.active_project_publisher.publish(msg)

            self.logger.info("Project successfully set to: {}.".format(project))
            response.success = True
        else:
            self.logger.error("Project does not exist: {}.".format(project))
            response.success = False
        return response


def main(args=None):
    rclpy.init(args=args)

    project_manager_node = ProjectManagerNode()

    rclpy.spin(project_manager_node)
    rclpy.shutdown()


if __name__ == '__main__':
    main()
