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
    wget \
    ros-iron-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/Kitware/CMake/releases/download/v3.26.3/cmake-3.26.3-linux-x86_64.tar.gz

RUN tar xf cmake-3.26.3-linux-x86_64.tar.gz

WORKDIR /app/ros2_ws

# Set the Cyclone DDS configuration
COPY ros2_ws/src/shared_packages/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Build interfaces
COPY ros2_ws/src/mtms_packages/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces/
COPY ros2_ws/src/shared_packages/interfaces/eeg_msgs src/shared_interfaces/eeg_msgs/
COPY ros2_ws/src/shared_packages/interfaces/mep_interfaces src/shared_interfaces/mep_interfaces/
COPY ros2_ws/src/shared_packages/interfaces/mtms_trial_interfaces src/shared_interfaces/mtms_trial_interfaces/
COPY ros2_ws/src/shared_packages/interfaces/targeting_msgs src/shared_interfaces/targeting_msgs/

RUN source /opt/ros/iron/setup.bash && colcon build


# Copy efield packages
COPY ros2_ws/src/mtms_packages/targeting/efield src/targeting/efield/

# Required so efield and efield_library can find their dependencies
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/app/ros2_ws/src/mtms_packages/targeting/efield/efield_libraries/lib"
ENV PATH="/cmake-3.26.3-linux-x86_64/bin:${PATH}"


# Build efield library
RUN cd src/targeting/efield/efield_libraries && cmake . && make

# Build efield packages
RUN source /opt/ros/iron/setup.bash && colcon build --packages-select efield


COPY ros2_ws/src/mtms_packages/targeting/efield/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
