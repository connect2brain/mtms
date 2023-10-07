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

    logger = LaunchConfiguration("log-level")
    safe_mode = LaunchConfiguration("safe-mode")

    node_executables = [
        "run_fpga",

        "start_device_handler",
        "stop_device_handler",
        "start_session_handler",
        "stop_session_handler",

        "allow_stimulation_handler",
        "allow_trigger_out_handler",

        "pulse_handler",
        "trigger_out_handler",
        "charge_handler",
        "discharge_handler",

        "event_trigger_handler",
        "feedback_monitor_bridge",
        "system_state_bridge",
        "settings_handler"
    ]

    for node_executable in node_executables:
        node = Node(
            package="mtms_device_bridge",
            executable=node_executable,
            parameters=[
                {
                    "safe-mode": safe_mode
                }
            ],
            arguments=['--ros-args', '--log-level', logger]
        )
        ld.add_action(node)

    ld.add_action(log_arg)

    return ld
