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
COPY ros2_ws/src/interfaces/experiment_interfaces src/interfaces/experiment_interfaces/
COPY ros2_ws/src/interfaces/eeg_interfaces src/interfaces/eeg_interfaces/
COPY ros2_ws/src/interfaces/event_interfaces src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/mep_interfaces src/interfaces/mep_interfaces/
COPY ros2_ws/src/interfaces/mtms_device_interfaces src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/interfaces/targeting_interfaces src/interfaces/targeting_interfaces/
COPY ros2_ws/src/interfaces/utility_interfaces src/interfaces/utility_interfaces/

RUN source /opt/ros/iron/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/experiment/stimulus_performer src/experiment/stimulus_performer/
RUN source /opt/ros/iron/setup.bash && colcon build --symlink-install --packages-select stimulus_performer

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/experiment/stimulus_performer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
