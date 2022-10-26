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

    logger = LaunchConfiguration("log-level")

    node_executables = [
        "run_fpga",

        "start_device_handler",
        "stop_device_handler",
        "start_experiment_handler",
        "stop_experiment_handler",

        "pulse_handler",
        "signal_out_handler",
        "charge_handler",
        "discharge_handler",

        "event_trigger_handler",
        "feedback_monitor_bridge",
        "system_state_bridge",
        "disable_checks_handler",
        "settings_handler"
    ]

    for node_executable in node_executables:
        node = Node(
            package="fpga_bridge",
            executable=node_executable,
            arguments=['--ros-args', '--log-level', logger]
        )
        ld.add_action(node)

    ld.add_action(log_arg)

    return ld
