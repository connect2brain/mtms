FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Setup environment

ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/interfaces ros2_ws/src/interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && colcon build

# Copy scripts

WORKDIR /app
COPY ros2_ws/src/other/bag_exporter/bag_exporter.py .

# Copy entrypoint

WORKDIR /app
COPY ros2_ws/src/other/bag_exporter/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
