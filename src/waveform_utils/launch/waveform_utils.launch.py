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

    ramp_down_timings_arg = DeclareLaunchArgument(
        "ramp-down-timings",
        description="Ramp-down timings in ticks",
    )

    logger = LaunchConfiguration("log-level")
    ramp_down_timings = LaunchConfiguration("ramp-down-timings")

    node_executables = [
        "get_default_waveform",
        "get_multipulse_waveforms",
        "reverse_polarity",
    ]

    for node_executable in node_executables:
        node = Node(
            package="waveform_utils",
            executable=node_executable,
            namespace="mtms",
            arguments=['--ros-args', '--log-level', logger],
            parameters=[{'ramp_down_timings': ramp_down_timings}]
        )
        ld.add_action(node)

    ld.add_action(log_arg)
    ld.add_action(ramp_down_timings_arg)

    return ld
