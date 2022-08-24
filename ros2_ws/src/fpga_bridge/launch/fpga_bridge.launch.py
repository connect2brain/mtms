from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    ld = LaunchDescription()

    log_arg = DeclareLaunchArgument(
        "log_level",
        default_value=["info"],
        description="Logging level",
    )

    logger = LaunchConfiguration("log_level")

    node_executables = [
        "run_fpga",
        #"safety_monitor_bridge",
        #"discharge_controller_bridge",
        "start_device_handler",
        "stop_device_handler",
        "start_experiment_handler",
        "stop_experiment_handler",
        "stimulation_pulse_event_handler",
        "trigger_out_event_handler",
        "charge_event_handler",
        "discharge_event_handler",
        "event_trigger_handler",
        "feedback_monitor_bridge",
        "system_state_monitor_bridge",
        "status_monitor_bridge",
        "disable_checks_handler"
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
