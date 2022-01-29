from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    ld = LaunchDescription()

    add_target_node = Node(
        package="planner",
        executable="add_target"
    )

    remove_target_node = Node(
        package="planner",
        executable="remove_target"
    )

    toggle_select_node = Node(
        package="planner",
        executable="toggle_select"
    )

    ld.add_action(add_target_node)
    ld.add_action(remove_target_node)
    ld.add_action(toggle_select_node)
    return ld
