FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/other/topic_frequency ros2_ws/src/other/topic_frequency/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && colcon build

# Copy entrypoint

COPY ros2_ws/src/other/topic_frequency/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
