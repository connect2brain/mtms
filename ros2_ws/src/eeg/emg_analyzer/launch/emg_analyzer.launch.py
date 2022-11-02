from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    ld = LaunchDescription()

    log_arg = DeclareLaunchArgument(
        "log-level",
        description="Logging level",
    )

    sampling_frequency_arg = DeclareLaunchArgument(
        "sampling-frequency",
        description="Sampling frequency",
    )

    logger = LaunchConfiguration("log-level")

    node_executables = [
        "analyze_mep",
    ]

    for node_executable in node_executables:
        node = Node(
            package="emg_analyzer",
            executable=node_executable,
            parameters=[
                {
                    "sampling_frequency": LaunchConfiguration("sampling-frequency"),
                }
            ],
            arguments=['--ros-args', '--log-level', logger]
        )
        ld.add_action(node)

    ld.add_action(log_arg)

    return ld
