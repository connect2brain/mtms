FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace for planner

WORKDIR /app
COPY ros2_ws/src/realtime/eeg_simulator ros2_ws/src/realtime/eeg_simulator/
COPY ros2_ws/src/interfaces/mtms_interfaces ros2_ws/src/interfaces/mtms_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/realtime/eeg_simulator/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
