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
COPY ros2_ws/src/experiment/trial_validator ros2_ws/src/experiment/trial_validator/

COPY ros2_ws/src/interfaces/experiment_interfaces ros2_ws/src/interfaces/experiment_interfaces/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/
COPY ros2_ws/src/interfaces/event_interfaces ros2_ws/src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/mep_interfaces ros2_ws/src/interfaces/mep_interfaces/
COPY ros2_ws/src/interfaces/mtms_device_interfaces ros2_ws/src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/interfaces/targeting_interfaces ros2_ws/src/interfaces/targeting_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/experiment/trial_validator/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
