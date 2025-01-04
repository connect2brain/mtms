# Note: NI FPGA drivers do not work in Docker as of the last time it was tried, so this Dockerfile is work in progress.

FROM osrf/ros:iron-desktop@sha256:eddad5f79aa7e888af29793c11f0ee44280c656b74dba0d37b2f48a0aa57352e

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    wget \
    unzip \
    ros-iron-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /build

# FPGA dependencies, instructions from https://www.ni.com/fi-fi/support/documentation/supplemental/18/downloading-and-installing-ni-driver-software-on-linux-desktop.html
RUN wget https://download.ni.com/support/softlib/MasterRepository/LinuxDrivers2022Q2/NILinux2022Q2DeviceDrivers.zip
RUN unzip NILinux2022Q2DeviceDrivers.zip

RUN apt-get dist-upgrade -y

# You MIGHT need to run this command on your host PC
# touch /usr/src/linux-headers-$(uname -r)/include/config/modversions.h

RUN apt-get install /build/NILinux2022Q2DeviceDrivers/ni-ubuntu2004firstlook-drivers-2022Q2.deb
RUN apt-get update
RUN apt-get install ni-rseries ni-fpga-interface -y
RUN dkms autoinstall


RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Set the Cyclone DDS configuration
COPY ros2_ws/src/shared_packages/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/mtms_packages/mtms/mtms_device_bridge ros2_ws/src/mtms_packages/mtms/mtms_device_bridge/

COPY ros2_ws/src/mtms_packages/interfaces/mtms_device_interfaces ros2_ws/src/mtms_packages/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/event_msgs ros2_ws/src/mtms_packages/interfaces/event_msgs/
COPY ros2_ws/src/mtms_packages/interfaces/waveform_msgs src/interfaces/waveform_msgs/

COPY ros2_ws/src/shared_packages/interfaces/system_interfaces ros2_ws/src/shared_packages/interfaces/system_interfaces/
COPY ros2_ws/src/shared_packages/realtime_utils ros2_ws/src/shared_packages/realtime_utils/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && \
    colcon build

# Copy entrypoint

COPY ros2_ws/src/mtms_packages/mtms/mtms_device_bridge/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

COPY bitfiles/ /app/ros2_ws/bitfiles/

ENV BITFILE_DIRECTORY=/app/ros2_ws/bitfiles/

ENTRYPOINT ["./ros_entrypoint.sh"]
