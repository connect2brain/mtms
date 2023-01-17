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
        description="Pre processor type",
    )
    pre_processor_script_arg = DeclareLaunchArgument(
        "preprocessor-script",
        description="Pre processor script path",
    )

    stimulus_presenter_type_arg = DeclareLaunchArgument(
        "stimulus-presenter-type",
        description="stimulus_presenter type",
    )
    stimulus_presenter_script_arg = DeclareLaunchArgument(
        "stimulus-presenter-script",
        description="stimulus_presenter script path",
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
        ),
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
        ),
        Node(
            package="eeg_processor",
            executable="stimulus_presenter",
            name="stimulus_presenter",
            output="screen",
            emulate_tty=True,
            parameters=[
                {
                    "processor_type": LaunchConfiguration("stimulus-presenter-type"),
                    "processor_script": LaunchConfiguration("stimulus-presenter-script"),
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
    ld.add_action(processor_type_arg)
    ld.add_action(processor_script_arg)
    ld.add_action(stimulus_presenter_script_arg)
    ld.add_action(stimulus_presenter_type_arg)

    return ld
