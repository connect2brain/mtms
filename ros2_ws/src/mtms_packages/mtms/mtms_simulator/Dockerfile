FROM osrf/ros:iron-desktop@sha256:eddad5f79aa7e888af29793c11f0ee44280c656b74dba0d37b2f48a0aa57352e

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

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY ros2_ws/src/mtms_packages/interfaces/mtms_device_interfaces src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/event_msgs src/interfaces/event_msgs/
COPY ros2_ws/src/mtms_packages/interfaces/waveform_msgs src/interfaces/waveform_msgs/

COPY ros2_ws/src/shared_packages/interfaces/system_interfaces src/shared_interfaces/system_interfaces/

RUN source /opt/ros/iron/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/mtms_packages/simulators/mtms_simulator/ ros2_ws/src/mtms_packages/simulators/mtms_simulator/
RUN source /opt/ros/iron/setup.bash && colcon build --symlink-install --packages-select mtms_simulator

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/mtms_packages/simulators/mtms_simulator/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
