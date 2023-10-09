import time
from threading import Event

import numpy as np

from experiment_interfaces.msg import Trial
from experiment_interfaces.srv import ValidateTrial

from targeting_interfaces.srv import GetMaximumIntensity

import rclpy
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor

class TrialValidatorNode(Node):
    def __init__(self):
        super().__init__('trial_validator_node')

        self.logger = self.get_logger()

        # Needed to enable calling another service inside a service handler. See also using MultiThreadedExecutor.
        self.callback_group = ReentrantCallbackGroup()

        # Create service for validating trial.

        self.service = self.create_service(
            ValidateTrial,
            '/trial/validate',
            self.validate_trial_callback,
            callback_group=self.callback_group,
        )

        # Create service client for targeting.

        self.get_maximum_intensity_client = self.create_client(GetMaximumIntensity, '/targeting/get_maximum_intensity', callback_group=self.callback_group)
        while not self.get_maximum_intensity_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /targeting/get_maximum_intensity not available, waiting...')

    def async_service_call(self, client, request):
        call_service_event = Event()
        response_value = [None]

        def service_call_callback(future):
            nonlocal response_value
            response_value[0] = future.result()
            call_service_event.set()

        service_call_future = client.call_async(request)
        service_call_future.add_done_callback(service_call_callback)

        # Wait for the service call to complete
        call_service_event.wait()

        response = response_value[0]
        return response

    def get_maximum_intensity(self, target):
        request = GetMaximumIntensity.Request()
        request.target = target

        response = self.async_service_call(self.get_maximum_intensity_client, request)

        assert response.success, "Get maximum intensity call failed."

        return response.maximum_intensity

    def validate_trial_callback(self, request, response):
        stimuli = request.trial.stimuli

        assert len(stimuli) == 1, "Multiple stimuli per trial not supported yet!"

        stimulus = stimuli[0]

        target = stimulus.target
        intensity = stimulus.intensity

        self.get_logger().info('Validation requested for target at (x, y, angle) = ({}, {}, {}) with intensity {} V/m.'.format(
            target.displacement_x,
            target.displacement_y,
            target.rotation_angle,
            intensity,
        ))
        max_intensity = self.get_maximum_intensity(target)

        is_trial_valid = intensity <= max_intensity

        self.get_logger().info('Maximum intensity for the requested target: {} V/m'.format(max_intensity))
        self.get_logger().info('Requested trial is {}.'.format('valid' if is_trial_valid else 'invalid'))

        response.is_trial_valid = is_trial_valid
        response.success = True

        return response


def main(args=None):
    rclpy.init(args=args)

    trial_validator_node = TrialValidatorNode()

    # Needed to enable calling another service within a service handler. See also using ReentrantCallbackGroup.
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(trial_validator_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
