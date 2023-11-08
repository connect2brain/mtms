import numpy as np

import rclpy
from rclpy.node import Node

from utility_interfaces.srv import GetNextId

class IdAssignerNode(Node):

    def __init__(self):
        super().__init__('id_assigner_node')

        self.logger = self.get_logger()

        # Create service for assigning the next ID.

        self.service = self.create_service(
            GetNextId,
            '/utility/get_next_id',
            self.get_next_id_callback,
        )
        self.id = 0

    def get_next_id_callback(self, request, response):
        response.id = self.id
        response.success = True

        self.get_logger().info('Assigned the ID: {}'.format(response.id))

        self.id += 1

        return response

def main(args=None):
    rclpy.init(args=args)

    id_assigner_node = IdAssignerNode()

    rclpy.spin(id_assigner_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
