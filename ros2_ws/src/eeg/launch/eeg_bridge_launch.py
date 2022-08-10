from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package="eeg",
            executable="eeg_bridge",
            name="eeg_bridge",
            output="screen",
            emulate_tty=True,
            parameters=[
                {"sampling_frequency": 5000.0} # Must be float
            ]
        )
    ])
