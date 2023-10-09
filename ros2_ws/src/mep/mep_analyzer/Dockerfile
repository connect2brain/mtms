FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY ros2_ws/src/interfaces/eeg_interfaces src/interfaces/eeg_interfaces/
COPY ros2_ws/src/interfaces/mep_interfaces src/interfaces/mep_interfaces/
RUN source /opt/ros/iron/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/mep/mep_analyzer src/mep/mep_analyzer/
RUN source /opt/ros/iron/setup.bash && colcon build --symlink-install --packages-select mep_analyzer

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/mep/mep_analyzer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
