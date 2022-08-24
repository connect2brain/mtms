from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    ld = LaunchDescription()

    nodes = [
        Node(
            package="fpga_bridge",
            executable="run_fpga"
        ),
        #Node(
        #    package="fpga_bridge",
        #    executable="safety_monitor_bridge"
        #),
        #Node(
        #    package="fpga_bridge",
        #    executable="discharge_controller_bridge"
        #),
        Node(
            package="fpga_bridge",
            executable="start_device_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="stop_device_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="start_experiment_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="stop_experiment_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="stimulation_pulse_event_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="trigger_out_event_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="charge_event_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="discharge_event_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="event_trigger_handler"
        ),
        Node(
            package="fpga_bridge",
            executable="feedback_monitor_bridge"
        ),
        Node(
            package="fpga_bridge",
            executable="system_state_monitor_bridge"
        ),
        Node(
            package="fpga_bridge",
            executable="disable_checks_handler"
        )
    ]

    for node in nodes:
        ld.add_action(node)

    return ld
