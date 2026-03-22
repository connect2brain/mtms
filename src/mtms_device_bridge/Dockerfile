# Note: NI FPGA drivers do not work in Docker as of the last time it was tried, so this Dockerfile is work in progress.

FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    wget \
    unzip \
    ros-jazzy-rmw-cyclonedds-cpp && \
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
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace

WORKDIR /app
COPY src/mtms_device_bridge src/mtms_device_bridge/

COPY interfaces/mtms_device_interfaces interfaces/mtms_device_interfaces/
COPY interfaces/mtms_event_interfaces interfaces/mtms_event_interfaces/
COPY interfaces/mtms_waveform_interfaces interfaces/mtms_waveform_interfaces/

COPY interfaces/mtms_system_interfaces interfaces/mtms_system_interfaces/
COPY src/realtime_utils src/realtime_utils/

# Build ROS packages

WORKDIR /app
RUN source /opt/ros/jazzy/setup.bash && \
    colcon build

# Copy entrypoint

COPY src/mtms_device_bridge/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

COPY bitfiles/ /app/bitfiles/

ENV BITFILE_DIRECTORY=/app/bitfiles/

ENTRYPOINT ["./ros_entrypoint.sh"]
