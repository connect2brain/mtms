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

    processor_type_arg = DeclareLaunchArgument(
        "processor-type",
        default_value=["python"],
        description="Processor type",
    )

    processor_script_arg = DeclareLaunchArgument(
        "processor-script",
        default_value=[""],
        description="Processor script",
    )


    logger = LaunchConfiguration("log_level")

    node = Node(
        package="data_processor",
        executable="data_processor",
        name="data_processor",
        output="screen",
        emulate_tty=True,
        parameters=[
            {
                "processor_type": LaunchConfiguration("processor-type"),
                "processor_script": LaunchConfiguration("processor-script")
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )
    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(processor_type_arg)
    ld.add_action(processor_script_arg)

    return ld
