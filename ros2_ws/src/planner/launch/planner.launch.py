from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    ld = LaunchDescription()

    nodes = [
        Node(
            package="planner",
            executable="add_target"
        ),
        Node(
            package="planner",
            executable="remove_target"
        ),
        Node(
            package="planner",
            executable="toggle_select_target"
        ),
        Node(
            package="planner",
            executable="set_target"
        ),
        Node(
            package="planner",
            executable="rename_target"
        ),
        Node(
            package="planner",
            executable="toggle_visible"
        ),
        Node(
            package="planner",
            executable="change_comment"
        ),
        Node(
            package="planner",
            executable="toggle_navigation"
        ),
        Node(
            package="planner",
            executable="clear_state"
        ),
        Node(
            package="planner",
            executable="change_target_index"
        ),
        Node(
            package="planner",
            executable="add_pulse_sequence"
        ),
        Node(
            package="planner",
            executable="rename_pulse_sequence"
        ),
        Node(
            package="planner",
            executable="toggle_select_pulse_sequence"
        ),
        Node(
            package="planner",
            executable="remove_pulse_sequence"
        ),
        Node(
            package="planner",
            executable="remove_pulse"
        )
    ]

    for node in nodes:
        ld.add_action(node)

    return ld
