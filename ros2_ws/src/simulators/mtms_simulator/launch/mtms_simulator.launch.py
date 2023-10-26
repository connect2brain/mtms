from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    log_arg = DeclareLaunchArgument(
            "log-level",
            description="Logging level",
            default_value="info"
        )
    logger = LaunchConfiguration("log-level")

    node = Node(
        package="mtms_simulator",
        executable="mtms_simulator",
        arguments=["--ros-args", "--log-level", logger]
    )

    return LaunchDescription([
        log_arg,
        node,
    ])
