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

    coil_array_arg = DeclareLaunchArgument(
        "coil-array",
        description="Coil array",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
            package="targeting",
            executable="targeting",
            name="targeting",
            parameters=[
                {
                    "coil_array": LaunchConfiguration("coil-array"),
                },
            ],
            output="screen",
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', logger]
        )

    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(coil_array_arg)

    return ld
