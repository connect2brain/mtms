import rclpy
from rclpy.node import Node
from neuronavigation_interfaces.srv import VisualizeTargets

class TargetVisualizer(Node):
    def __init__(self, callback_group):
        super().__init__("target_visualizer")

        # Create service for visualizing targets.
        self.visualize_targets_service = self.create_service(
            VisualizeTargets,
            "/neuronavigation/visualize/targets",
            self.visualize_targets,
            callback_group=callback_group,
        )

    def set_callback__set_vector_field(self, callback):
        self._set_vector_field = callback

    def visualize_targets(self, request, response):
        print('Received request to visualize targets, # of targets: {}'.format(len(request.targets)))

        targets = request.targets
        is_ordered = request.is_ordered

        if is_ordered and len(targets) > 1:
            # If the targets are ordered, create a color gradient from blue to red.
            color_start = (0, 0, 1.0)
            color_end = (1.0, 0, 0)

            colors = [tuple(c1 + (c2 - c1) * i / (len(targets) - 1) for c1, c2 in zip(color_start, color_end)) for i in range(len(targets))]
        else:
            # If the targets are not ordered, color them all blue.
            colors = [(0, 0, 1.0)] * len(targets)

        vector_field = []
        for target, color in zip(targets, colors):
            # Visualize each target 15 mm below the coil centerpoint.
            position = [target.displacement_x, target.displacement_y, -15]
            orientation = [0, 0, target.rotation_angle]
            length = target.intensity / 100

            vector_field.append({
                'position': position,
                'orientation': orientation,
                'color': color,
                'length': length,
            })

        self._set_vector_field(vector_field)

        response.success = True
        return response
