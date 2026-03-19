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

# Copy ROS workspace

WORKDIR /app

# ROS interfaces
COPY interfaces/event_interfaces interfaces/event_interfaces/
COPY interfaces/waveform_interfaces interfaces/waveform_interfaces/
COPY interfaces/neuronavigation_interfaces interfaces/neuronavigation_interfaces/
COPY interfaces/system_interfaces interfaces/system_interfaces/
COPY interfaces/targeting_interfaces interfaces/targeting_interfaces/

COPY interfaces/targeting_interfaces interfaces/targeting_interfaces/

RUN source /opt/ros/jazzy/setup.bash && colcon build

# ROS node
COPY src/stimulation_allower src/mtms/stimulation_allower/
RUN source /opt/ros/jazzy/setup.bash && colcon build --symlink-install --packages-select stimulation_allower

# Copy entrypoint

COPY src/stimulation_allower/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
