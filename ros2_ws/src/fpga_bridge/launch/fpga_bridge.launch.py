from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    ld = LaunchDescription()

    run_fpga_node = Node(
        package="fpga_bridge",
        executable="run_fpga"
    )

    safety_monitor_bridge_node = Node(
        package="fpga_bridge",
        executable="safety_monitor_bridge"
    )

    discharge_controller_bridge_node = Node(
        package="fpga_bridge",
        executable="discharge_controller_bridge"
    )

    set_power_handler_node = Node(
        package="fpga_bridge",
        executable="set_power_handler"
    )

    stimulation_pulse_event_handler_node = Node(
        package="fpga_bridge",
        executable="stimulation_pulse_event_handler"
    )

    trigger_out_pulse_event_handler_node = Node(
        package="fpga_bridge",
        executable="trigger_out_pulse_event_handler"
    )

    charge_event_handler_node = Node(
        package="fpga_bridge",
        executable="charge_event_handler"
    )

    discharge_event_handler_node = Node(
        package="fpga_bridge",
        executable="discharge_event_handler"
    )

    event_trigger_handler_node = Node(
        package="fpga_bridge",
        executable="event_trigger_handler"
    )

    ld.add_action(run_fpga_node)
    ld.add_action(safety_monitor_bridge_node)
    ld.add_action(discharge_controller_bridge_node)
    ld.add_action(stimulation_pulse_event_handler_node)
    ld.add_action(trigger_out_pulse_event_handler_node)
    ld.add_action(charge_event_handler_node)
    ld.add_action(discharge_event_handler_node)
    ld.add_action(event_trigger_handler_node)
    return ld
