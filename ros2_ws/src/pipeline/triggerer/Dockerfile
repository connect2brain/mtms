FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    python3-pip \
    libserialport-dev \
    ros-iron-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY ros2_ws/src/interfaces/event_interfaces src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/system_interfaces src/interfaces/system_interfaces/
RUN source /opt/ros/iron/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/pipeline/triggerer src/pipeline/triggerer/
RUN source /opt/ros/iron/setup.bash && colcon build --symlink-install --packages-select triggerer

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/pipeline/triggerer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
