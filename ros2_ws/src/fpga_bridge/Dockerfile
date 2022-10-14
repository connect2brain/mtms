FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies
WORKDIR /build

RUN apt-get update
RUN apt-get install wget unzip

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

# Copy ROS workspace for fpga bridge

WORKDIR /app
COPY ros2_ws/src/fpga_bridge ros2_ws/src/fpga_bridge/
COPY ros2_ws/src/fpga_interfaces ros2_ws/src/fpga_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build

# Copy entrypoint

COPY ros2_ws/src/fpga_bridge/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]