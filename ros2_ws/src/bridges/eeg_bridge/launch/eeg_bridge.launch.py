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

    number_of_eeg_channels_amplifier_1_arg = DeclareLaunchArgument(
        "number-of-eeg-channels-amplifier-1",
        description="EEG channel count for amplifier 1",
    )

    number_of_emg_channels_amplifier_1_arg = DeclareLaunchArgument(
        "number-of-emg-channels-amplifier-1",
        description="EMG channel count for amplifier 1",
    )

    number_of_eeg_channels_amplifier_2_arg = DeclareLaunchArgument(
        "number-of-eeg-channels-amplifier-2",
        description="EEG channel count for amplifier 2",
    )

    number_of_emg_channels_amplifier_2_arg = DeclareLaunchArgument(
        "number-of-emg-channels-amplifier-2",
        description="EMG channel count for amplifier 2",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
            package="eeg_bridge",
            executable="eeg_bridge",
            name="eeg_bridge",
            parameters=[
                {
                    "port": LaunchConfiguration("port"),
                    "number_of_eeg_channels_amplifier_1": LaunchConfiguration("number-of-eeg-channels-amplifier-1"),
                    "number_of_emg_channels_amplifier_1": LaunchConfiguration("number-of-emg-channels-amplifier-1"),
                    "number_of_eeg_channels_amplifier_2": LaunchConfiguration("number-of-eeg-channels-amplifier-2"),
                    "number_of_emg_channels_amplifier_2": LaunchConfiguration("number-of-emg-channels-amplifier-2"),
                }
            ],
            output="screen",
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', logger]
        )

    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(port_arg)
    ld.add_action(number_of_eeg_channels_amplifier_1_arg)
    ld.add_action(number_of_emg_channels_amplifier_1_arg)
    ld.add_action(number_of_eeg_channels_amplifier_2_arg)
    ld.add_action(number_of_emg_channels_amplifier_2_arg)

    return ld
