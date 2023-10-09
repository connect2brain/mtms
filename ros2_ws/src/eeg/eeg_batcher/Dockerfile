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

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/eeg/eeg_batcher ros2_ws/src/eeg/eeg_batcher/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/eeg/eeg_batcher/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
