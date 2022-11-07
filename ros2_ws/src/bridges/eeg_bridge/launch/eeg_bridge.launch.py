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

    eeg_channels_primary_amplifier_arg = DeclareLaunchArgument(
        "eeg-channels-primary-amplifier",
        description="EEG channel count for primary amplifier",
    )

    emg_channels_primary_amplifier_arg = DeclareLaunchArgument(
        "emg-channels-primary-amplifier",
        description="EMG channel count for primary amplifier",
    )

    eeg_channels_secondary_amplifier_arg = DeclareLaunchArgument(
        "eeg-channels-secondary-amplifier",
        description="EEG channel count for secondary amplifier",
    )

    emg_channels_secondary_amplifier_arg = DeclareLaunchArgument(
        "emg-channels-secondary-amplifier",
        description="EMG channel count for secondary amplifier",
    )

    logger = LaunchConfiguration("log-level")

    node = Node(
            package="eeg_bridge",
            executable="eeg_bridge",
            name="eeg_bridge",
            parameters=[
                {
                    "port": LaunchConfiguration("port"),
                    "eeg_channels_primary_amplifier": LaunchConfiguration("eeg-channels-primary-amplifier"),
                    "emg_channels_primary_amplifier": LaunchConfiguration("emg-channels-primary-amplifier"),
                    "eeg_channels_secondary_amplifier": LaunchConfiguration("eeg-channels-secondary-amplifier"),
                    "emg_channels_secondary_amplifier": LaunchConfiguration("emg-channels-secondary-amplifier"),
                }
            ],
            output="screen",
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', logger]
        )

    ld.add_action(node)
    ld.add_action(log_arg)
    ld.add_action(port_arg)
    ld.add_action(eeg_channels_primary_amplifier_arg)
    ld.add_action(emg_channels_primary_amplifier_arg)
    ld.add_action(eeg_channels_secondary_amplifier_arg)
    ld.add_action(emg_channels_secondary_amplifier_arg)

    return ld
