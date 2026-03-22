FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    ros-jazzy-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app

# ROS interfaces
COPY interfaces/mtms_event_interfaces interfaces/mtms_event_interfaces/
COPY interfaces/mtms_targeting_interfaces interfaces/mtms_targeting_interfaces/
COPY interfaces/mtms_waveform_interfaces interfaces/mtms_waveform_interfaces/

COPY interfaces/mtms_targeting_interfaces interfaces/mtms_targeting_interfaces/

RUN source /opt/ros/jazzy/setup.bash && colcon build

# ROS node
COPY src/targeting src/targeting/
RUN source /opt/ros/jazzy/setup.bash && colcon build --symlink-install --packages-select targeting

# Copy data files
COPY src/targeting/data data

# Copy entrypoint

COPY src/targeting/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
