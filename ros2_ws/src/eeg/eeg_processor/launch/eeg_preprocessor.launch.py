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
    pre_processor_type_arg = DeclareLaunchArgument(
        "preprocessor-type",
        description="Preprocessor type",
    )
    pre_processor_script_arg = DeclareLaunchArgument(
        "preprocessor-script",
        description="Preprocessor script path",
    )

    logger = LaunchConfiguration("log-level")

    nodes = [
        Node(
            package="eeg_processor",
            executable="eeg_preprocessor",
            name="eeg_preprocessor",
            output="screen",
            emulate_tty=True,
            parameters=[
                {
                    "processor_type": LaunchConfiguration("preprocessor-type"),
                    "processor_script": LaunchConfiguration("preprocessor-script"),
                }
            ],
            arguments=['--ros-args', '--log-level', logger]
        )
    ]
    for node in nodes:
        ld.add_action(node)

    ld.add_action(log_arg)
    ld.add_action(pre_processor_type_arg)
    ld.add_action(pre_processor_script_arg)

    return ld
