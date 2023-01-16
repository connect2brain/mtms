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
        "pre-processor-type",
        description="Pre processor type",
    )
    pre_processor_script_arg = DeclareLaunchArgument(
        "pre-processor-script",
        description="Pre processor script path",
    )

    visualizer_type_arg = DeclareLaunchArgument(
        "visualizer-type",
        description="Visualizer type",
    )
    visualizer_script_arg = DeclareLaunchArgument(
        "visualizer-script",
        description="Visualizer script path",
    )

    processor_type_arg = DeclareLaunchArgument(
        "processor-type",
        description="Processor type",
    )

    processor_script_arg = DeclareLaunchArgument(
        "processor-script",
        description="Processor script",
    )

    logger = LaunchConfiguration("log-level")

    nodes = [
        Node(
            package="eeg_processor",
            executable="eeg_processor",
            name="eeg_processor",
            output="screen",
            emulate_tty=True,
            parameters=[
                {
                    "processor_type": LaunchConfiguration("processor-type"),
                    "processor_script": LaunchConfiguration("processor-script"),
                }
            ],
            arguments=['--ros-args', '--log-level', logger]
        )
    ]
    for node in nodes:
        ld.add_action(node)

    ld.add_action(log_arg)
    ld.add_action(processor_type_arg)
    ld.add_action(processor_script_arg)
    ld.add_action(visualizer_script_arg)
    ld.add_action(visualizer_type_arg)

    return ld
