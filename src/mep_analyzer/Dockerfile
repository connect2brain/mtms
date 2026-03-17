FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    ros-jazzy-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY src/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY src/interfaces/eeg_msgs src/shared_interfaces/eeg_msgs/
COPY src/interfaces/mep_interfaces src/shared_interfaces/mep_interfaces/
COPY src/interfaces/system_interfaces src/shared_interfaces/system_interfaces/
RUN source /opt/ros/jazzy/setup.bash && colcon build

# ROS node
COPY src/mep_analyzer src/mep/mep_analyzer/
RUN source /opt/ros/jazzy/setup.bash && colcon build --symlink-install --packages-select mep_analyzer

# Copy entrypoint

WORKDIR /app

COPY src/mep_analyzer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
