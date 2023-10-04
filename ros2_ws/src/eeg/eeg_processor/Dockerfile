FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Note: Needed because one of PsychoPy's dependencies asks for keyboard input.
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies

WORKDIR /build

RUN apt-get update
RUN apt-get install -y \
    dos2unix \
    wget \
    python3-pip \
    ros-iron-rmw-cyclonedds-cpp

# PsychoPy and its dependencies
RUN apt-get install -y libgtk-3-dev
RUN pip3 install attrdict3 psychopy

# Note: Tensorflow is needed for mneflow, which is used for machine learning.
RUN pip3 install tensorflow

# HACK: Currently needed for mneflow to install successfully. Remove once the corresponding problem
#   in mneflow has been fixed.
ENV SKLEARN_ALLOW_DEPRECATED_SKLEARN_PACKAGE_INSTALL=True

RUN pip3 install git+https://github.com/zubara/mneflow.git
RUN apt-get install -y python3-tk

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/eeg/eeg_processor ros2_ws/src/eeg/eeg_processor/
COPY ros2_ws/src/interfaces/event_interfaces ros2_ws/src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/mtms_device_interfaces ros2_ws/src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && colcon build

# Create a directory for pipeline

RUN mkdir -p pipeline
RUN mkdir -p measurements

# Copy entrypoints

COPY ros2_ws/src/eeg/eeg_processor/eeg_processor_entrypoint.sh .
COPY ros2_ws/src/eeg/eeg_processor/eeg_preprocessor_entrypoint.sh .
COPY ros2_ws/src/eeg/eeg_processor/stimulus_presenter_entrypoint.sh .

RUN dos2unix *.sh
RUN chmod +x *.sh

ENV DOCKER=true

# Note: This entrypoint is overriden in docker-compose.yml for certain services.
ENTRYPOINT ["./eeg_processor_entrypoint.sh"]
