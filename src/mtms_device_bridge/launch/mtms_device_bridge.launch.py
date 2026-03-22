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

    safe_mode_arg = DeclareLaunchArgument(
        "safe-mode",
        description="Run in safe mode (boolean)",
    )

    channel_count_arg = DeclareLaunchArgument(
        "channel-count",
        description="The channel count of the mTMS device",
    )

    logger = LaunchConfiguration("log-level")
    safe_mode = LaunchConfiguration("safe-mode")
    channel_count = LaunchConfiguration("channel-count")

    node_executables = [
        "run_fpga",

        "start_device_handler",
        "stop_device_handler",
        "start_session_handler",
        "stop_session_handler",

        "allow_stimulation_handler",
        "allow_trigger_out_handler",

        "event_handler",
        "trigger_events_handler",

        "feedback_monitor_bridge",
        "system_state_bridge",
        "session_bridge",

        "settings_handler"
    ]

    for node_executable in node_executables:
        node = Node(
            package="mtms_device_bridge",
            executable=node_executable,
            namespace="mtms",
            parameters=[
                {
                    "safe-mode": safe_mode,
                    "channel-count": channel_count,
                }
            ],
            arguments=['--ros-args', '--log-level', logger, '--disable-external-lib-logs']
        )
        ld.add_action(node)

    ld.add_action(log_arg)

    return ld
