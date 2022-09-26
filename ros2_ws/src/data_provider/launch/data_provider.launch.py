from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    ld = LaunchDescription()

    log_arg = DeclareLaunchArgument(
        "log-level",
        default_value=["info"],
        description="Logging level",
    )

    data_file_arg = DeclareLaunchArgument(
        "data-file",
        default_value=[""],
        description="Data file from which to read data to provide",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
        package="data_provider",
        executable="data_provider",
        parameters=[
            {
                "data_file":
                    LaunchConfiguration("data-file")
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )
    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(data_file_arg)

    return ld
