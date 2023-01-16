FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace for eeg processor

WORKDIR /app
COPY ros2_ws/src/eeg/eeg_processor ros2_ws/src/eeg/eeg_processor/
COPY ros2_ws/src/interfaces/event_interfaces ros2_ws/src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/fpga_interfaces ros2_ws/src/interfaces/fpga_interfaces/
COPY ros2_ws/src/interfaces/mtms_interfaces ros2_ws/src/interfaces/mtms_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && colcon build

# Create a directory for hotswappable processors

RUN mkdir -p hotswappable_processors
RUN mkdir -p measurements

# Copy entrypoint

COPY ros2_ws/src/eeg/eeg_processor/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENV DOCKER=true

ENTRYPOINT ["./ros_entrypoint.sh"]
