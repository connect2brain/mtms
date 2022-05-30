import rclpy
from rclpy.node import Node

from mtms_interfaces.msg import PlannerState, Target
from std_msgs.msg import Bool, Float64, String
from mtms_interfaces.srv import StartPulseSequence


class StartPulseSequenceNode(Node):
    def __init__(self):
        super().__init__('start_pulse_sequence')
        self.create_service(StartPulseSequence, '/stimulation/start_experiment', self.start_pulse_sequence)

    def start_pulse_sequence(self, request, response):
        self.get_logger().info('Incoming request', request)
        print(request)
        response.success = True
        return response


def main():
    rclpy.init()

    start_sequence_node = StartPulseSequenceNode()

    rclpy.spin(start_sequence_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
