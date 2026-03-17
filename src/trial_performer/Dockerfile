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

WORKDIR /app

# ROS interfaces
COPY src/interfaces/event_msgs src/interfaces/event_msgs/
COPY src/interfaces/experiment_interfaces src/interfaces/experiment_interfaces/
COPY src/interfaces/mtms_device_interfaces src/interfaces/mtms_device_interfaces/
COPY src/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces/
COPY src/interfaces/targeting_services src/interfaces/targeting_services/
COPY src/interfaces/waveform_msgs src/interfaces/waveform_msgs/

COPY src/interfaces/eeg_msgs src/interfaces/eeg_msgs/
COPY src/interfaces/mep_interfaces src/interfaces/mep_interfaces/
COPY src/interfaces/system_interfaces src/interfaces/system_interfaces/
COPY src/interfaces/targeting_msgs src/interfaces/targeting_msgs/
COPY src/interfaces/mtms_trial_interfaces src/interfaces/mtms_trial_interfaces/

RUN source /opt/ros/jazzy/setup.bash && colcon build

# ROS node
COPY src/trial_performer src/trial_performer/
RUN source /opt/ros/jazzy/setup.bash && colcon build --symlink-install --packages-select trial_performer

# Copy entrypoint
COPY src/trial_performer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
