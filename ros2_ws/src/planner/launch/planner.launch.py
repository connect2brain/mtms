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

    set_target_node = Node(
        package="planner",
        executable="set_target"
    )

    rename_target_node = Node(
        package="planner",
        executable="rename_target"
    )

    toggle_visible_node = Node(
        package="planner",
        executable="toggle_visible"
    )

    change_comment_node = Node(
        package="planner",
        executable="change_comment"
    )

    toggle_navigation_node = Node(
        package="planner",
        executable="toggle_navigation"
    )

    clear_state_node = Node(
        package="planner",
        executable="clear_state"
    )

    ld.add_action(add_target_node)
    ld.add_action(remove_target_node)
    ld.add_action(toggle_select_node)
    ld.add_action(set_target_node)
    ld.add_action(rename_target_node)
    ld.add_action(toggle_visible_node)
    ld.add_action(change_comment_node)
    ld.add_action(toggle_navigation_node)
    ld.add_action(clear_state_node)
    return ld
