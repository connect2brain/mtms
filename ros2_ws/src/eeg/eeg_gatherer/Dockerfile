FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/eeg/eeg_gatherer ros2_ws/src/eeg/eeg_gatherer/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/eeg/eeg_gatherer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
