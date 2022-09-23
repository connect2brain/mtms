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

    data_size_arg = DeclareLaunchArgument(
        "data-size",
        default_value=[""],
        description="How many durations to calculate",
    )

    file_arg = DeclareLaunchArgument(
        "file",
        default_value=[""],
        description="Filename to store the durations at the end",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
        package="data_processor",
        executable="data_processor",
        name="data_processor",
        output="screen",
        emulate_tty=True,
        parameters=[
            {
                "data_size": LaunchConfiguration("data-size"),
                "file": LaunchConfiguration("file")
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )
    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(data_size_arg)
    ld.add_action(file_arg)

    return ld
