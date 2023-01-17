FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace for eeg bridge

WORKDIR /app
COPY ros2_ws/src/bridges/eeg_bridge ros2_ws/src/bridges/eeg_bridge/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/
COPY ros2_ws/src/interfaces/fpga_interfaces ros2_ws/src/interfaces/fpga_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/bridges/eeg_bridge/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]