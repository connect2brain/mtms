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
    server_index_arg = DeclareLaunchArgument(
        "server-index",
        default_value=["1"],
        description="Server index",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
        package="optitrack_bridge",
        executable="optitrack_bridge",
        name="optitrack_bridge",
        parameters=[
            {
                "server_index": LaunchConfiguration("server-index")
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )
    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(server_index_arg)

    return ld
