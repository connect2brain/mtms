from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    log_arg = DeclareLaunchArgument(
        "log-level",
        description="Logging level",
    )

    port_arg = DeclareLaunchArgument(
        "port",
        description="Port",
    )

    eeg_device_arg = DeclareLaunchArgument(
        "eeg-device",
        description="EEG device to use: neurone | turbolink",
    )

    turbolink_sampling_frequency_arg = DeclareLaunchArgument(
        "turbolink-sampling-frequency",
        description="Sampling frequency of a Turbolink device",
    )

    turbolink_eeg_channel_count_arg = DeclareLaunchArgument(
        "turbolink-eeg-channel-count",
        description="EEG channel count of a Turbolink device",
    )


    logger = LaunchConfiguration("log-level")
    node = Node(
            package="eeg_bridge",
            executable="eeg_bridge",
            name="eeg_bridge",
            parameters=[
                {
                    "port": LaunchConfiguration("port"),
                    "eeg-device": LaunchConfiguration("eeg-device"),
                    "turbolink-sampling-frequency": LaunchConfiguration("turbolink-sampling-frequency"),
                    "turbolink-eeg-channel-count": LaunchConfiguration("turbolink-eeg-channel-count")
                }
            ],
            output="screen",
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', logger]
        )

    return LaunchDescription([
        log_arg,
        port_arg,
        eeg_device_arg,
        turbolink_sampling_frequency_arg,
        turbolink_eeg_channel_count_arg,
        node
    ])
