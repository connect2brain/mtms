FROM osrf/ros:iron-desktop@sha256:eddad5f79aa7e888af29793c11f0ee44280c656b74dba0d37b2f48a0aa57352e

SHELL ["/bin/bash", "-c"]

# Pin to the latest stable version of ros-apt-source
ENV ROS_APT_SOURCE_VERSION=1.1.0
ENV ROS_DISTRO_CODENAME=jammy
ENV ROS_APT_DEB_NAME=ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${ROS_DISTRO_CODENAME}_all.deb
ENV ROS_APT_DEB_URL=https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/${ROS_APT_DEB_NAME}
# Safely remove existing ROS 2 APT sources and keys (if present)
RUN rm -f /etc/apt/sources.list.d/ros2-latest.list
RUN rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg
# Install essential tools for downloading and verifying packages
RUN apt-get update && apt-get install -y curl ca-certificates gnupg
# Download the pinned version of the ros2-apt-source .deb
RUN curl -sL -o /tmp/${ROS_APT_DEB_NAME} ${ROS_APT_DEB_URL}
# Verify the DEB's SHA256 checksum
RUN echo "1600cb8cc28258a39bffc1736a75bcbf52d1f2db371a4d020c1b187d2a5a083b  /tmp/${ROS_APT_DEB_NAME}" | sha256sum -c -
# Install the DEB via apt, update package list, and clean up
RUN apt-get update \
    && apt-get install -y /tmp/${ROS_APT_DEB_NAME} \
    && rm -f /tmp/${ROS_APT_DEB_NAME} \

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
COPY ros2_ws/src/shared_packages/interfaces/eeg_msgs src/shared_interfaces/eeg_msgs/
COPY ros2_ws/src/shared_packages/interfaces/mep_interfaces src/shared_interfaces/mep_interfaces/
RUN source /opt/ros/iron/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/mtms_packages/mep/eeg_gatherer src/mep/eeg_gatherer/
RUN source /opt/ros/iron/setup.bash && colcon build --symlink-install --packages-select eeg_gatherer

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/mtms_packages/mep/eeg_gatherer/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
