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

    topic_arg = DeclareLaunchArgument(
        "topic",
        description="Topic name",
    )

    logger = LaunchConfiguration("log-level")
    topic = LaunchConfiguration("topic")

    node = Node(
        package="topic_frequency",
        executable="topic_frequency",
        name="topic_frequency",
        parameters=[
            {
                "topic": topic
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )

    ld.add_action(node)
    ld.add_action(log_arg)

    return ld
