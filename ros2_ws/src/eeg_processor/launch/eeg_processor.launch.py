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

    loop_count_arg = DeclareLaunchArgument(
        "loop-count",
        default_value=[""],
        description="Processor script loop count",
    )

    file_arg = DeclareLaunchArgument(
        "file",
        default_value=[""],
        description="Processor script file name",
    )


    logger = LaunchConfiguration("log-level")

    node = Node(
        package="eeg_processor",
        executable="eeg_processor",
        name="eeg_processor",
        output="screen",
        emulate_tty=True,
        parameters=[
            {
                "processor_type": LaunchConfiguration("processor-type"),
                "processor_script": LaunchConfiguration("processor-script"),
                "loop_count": LaunchConfiguration("loop-count"),
                "file": LaunchConfiguration("file")
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )
    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(processor_type_arg)
    ld.add_action(processor_script_arg)
    ld.add_action(loop_count_arg)
    ld.add_action(file_arg)

    return ld
