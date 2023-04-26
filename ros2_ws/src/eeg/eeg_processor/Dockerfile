FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

WORKDIR /build

RUN apt-get update

# Note: Needed because one of PsychoPy's dependencies asks for keyboard input.
ENV DEBIAN_FRONTEND=noninteractive

# PsychoPy and its dependencies
RUN apt-get install -y dos2unix wget python3-pip libgtk-3-dev
RUN pip3 install attrdict3
RUN pip3 install psychopy

# HACK: Needed because the default version of NumPy is incompatible with PsychoPy.
RUN pip3 install --force-reinstall "numpy==1.20.3"

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/eeg/eeg_processor ros2_ws/src/eeg/eeg_processor/
COPY ros2_ws/src/interfaces/event_interfaces ros2_ws/src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/mtms_device_interfaces ros2_ws/src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && colcon build

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
