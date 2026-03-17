FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    ros-jazzy-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY ros2_ws/src/shared_packages/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY ros2_ws/src/mtms_packages/interfaces/experiment_interfaces src/interfaces/experiment_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/event_msgs src/interfaces/event_msgs/
COPY ros2_ws/src/mtms_packages/interfaces/mtms_device_interfaces src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/targeting_services src/interfaces/targeting_services/
COPY ros2_ws/src/mtms_packages/interfaces/waveform_msgs src/interfaces/waveform_msgs/

COPY ros2_ws/src/shared_packages/interfaces/eeg_msgs src/shared_interfaces/eeg_msgs/
COPY ros2_ws/src/shared_packages/interfaces/mep_interfaces src/shared_interfaces/mep_interfaces/
COPY ros2_ws/src/shared_packages/interfaces/targeting_msgs src/shared_interfaces/targeting_msgs/
COPY ros2_ws/src/shared_packages/interfaces/mtms_trial_interfaces src/shared_interfaces/mtms_trial_interfaces/

RUN source /opt/ros/jazzy/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/mtms_packages/experiment/trial_validator src/experiment/trial_validator/
RUN source /opt/ros/jazzy/setup.bash && colcon build --symlink-install --packages-select trial_validator

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/mtms_packages/experiment/trial_validator/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
