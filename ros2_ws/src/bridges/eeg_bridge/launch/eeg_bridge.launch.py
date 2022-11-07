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

    port_arg = DeclareLaunchArgument(
        "port",
        description="Port",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
            package="eeg_bridge",
            executable="eeg_bridge",
            name="eeg_bridge",
            parameters=[
                {
                    "port": LaunchConfiguration("port"),
                }
            ],
            output="screen",
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', logger]
        )

    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(port_arg)

    return ld
