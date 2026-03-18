from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    """
    Launch EEG bridge node.
    """
    log_arg = DeclareLaunchArgument(
        "log-level",
        default_value=["info"],
        description="Logging level",
    )

    eeg_port_arg = DeclareLaunchArgument(
        "eeg-port",
        default_value=["50001"],
        description="UDP port for EEG packets",
    )

    eeg_device_arg = DeclareLaunchArgument(
        "eeg-device",
        default_value=["neurone"],
        description="EEG device adapter type",
    )

    logger = LaunchConfiguration("log-level")
    eeg_port = LaunchConfiguration("eeg-port")
    eeg_device = LaunchConfiguration("eeg-device")
    node = Node(
            package="eeg_bridge",
            executable="eeg_bridge",
            name="eeg_bridge",
            output="screen",
            emulate_tty=True,
            parameters=[{
                "eeg_port": eeg_port,
                "eeg_device": eeg_device,
            }],
            arguments=['--ros-args', '--log-level', logger]
        )

    return LaunchDescription([
        log_arg,
        eeg_port_arg,
        eeg_device_arg,
        node
    ])
