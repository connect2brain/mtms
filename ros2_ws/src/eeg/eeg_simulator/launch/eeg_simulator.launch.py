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

    data_file_arg = DeclareLaunchArgument(
        "data-file",
        description="Data file from which to read data to provide",
    )

    sampling_frequency_arg = DeclareLaunchArgument(
        "sampling-frequency",
        description="Sampling frequency",
    )

    loop_arg = DeclareLaunchArgument(
        "loop",
        description="Loop through the data file",
    )

    eeg_channels_arg = DeclareLaunchArgument(
        "eeg-channels",
        description="Number of EEG channels",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
        package="eeg_simulator",
        executable="eeg_simulator",
        parameters=[
            {
                "data_file": LaunchConfiguration("data-file"),
                "sampling_frequency": LaunchConfiguration("sampling-frequency"),
                "loop": LaunchConfiguration("loop"),
                "eeg_channels": LaunchConfiguration("eeg-channels")
            }
        ],
        arguments=['--ros-args', '--log-level', logger]
    )
    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(data_file_arg)
    ld.add_action(loop_arg)
    ld.add_action(sampling_frequency_arg)
    ld.add_action(eeg_channels_arg)

    return ld
