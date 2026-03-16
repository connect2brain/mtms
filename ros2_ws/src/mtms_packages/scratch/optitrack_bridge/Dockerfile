FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Pin to the latest stable version of ros-apt-source
ENV ROS_APT_SOURCE_VERSION=1.1.0
# Remove existing ROS 2 APT sources and keyrings to prevent conflicts
RUN rm -f /etc/apt/sources.list.d/ros2-latest.list && \
    rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg
# Install curl and certificates required for downloading files securely
RUN apt-get update && apt-get install -y curl ca-certificates
# Use ROS_APT_SOURCE_VERSION and OS codename to build download URL and install the .deb
RUN . /etc/os-release && \
    curl -L -s -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${VERSION_CODENAME}_all.deb" && \
    apt-get update && \
    apt-get install -y /tmp/ros2-apt-source.deb && \
    rm -f /tmp/ros2-apt-source.deb

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY ros2_ws/src/shared_packages/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/mtms_packages/utility/optitrack_bridge ros2_ws/src/mtms_packages/utility/optitrack_bridge/
COPY ros2_ws/src/mtms_packages/interfaces/neuronavigation_interfaces ros2_ws/src/mtms_packages/interfaces/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/jazzy/setup.bash && \
    colcon build

# Copy entrypoint

COPY ros2_ws/src/mtms_packages/utility/optitrack_bridge/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
