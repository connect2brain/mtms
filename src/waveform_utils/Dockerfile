FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    libboost-dev \
    ros-jazzy-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY src/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY src/interfaces/event_msgs src/interfaces/event_msgs/
COPY src/interfaces/targeting_services src/interfaces/targeting_services/
COPY src/interfaces/waveform_msgs src/interfaces/waveform_msgs/

COPY src/interfaces/targeting_msgs src/shared_interfaces/targeting_msgs/

RUN source /opt/ros/jazzy/setup.bash && colcon build

# ROS node
COPY src/waveform_utils src/targeting/waveform_utils/
RUN source /opt/ros/jazzy/setup.bash && colcon build --symlink-install --packages-select waveform_utils

# Copy entrypoint

COPY src/waveform_utils/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
