FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

WORKDIR /build

RUN apt-get update

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get install -y dos2unix wget python3-pip python3-wxgtk4.0
RUN apt-get install -y libgtk-3-dev

RUN pip3 install attrdict3
RUN pip3 install psychopy

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

# Create a directory for hotswappable processors

RUN mkdir -p hotswappable_processors
RUN mkdir -p measurements

# Copy entrypoint

COPY ros2_ws/src/eeg/eeg_processor/ros_entrypoint.sh .
COPY ros2_ws/src/eeg/eeg_processor/preprocessor_ros_entrypoint.sh .
COPY ros2_ws/src/eeg/eeg_processor/stimulus_presenter_ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh stimulus_presenter_ros_entrypoint.sh preprocessor_ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh stimulus_presenter_ros_entrypoint.sh preprocessor_ros_entrypoint.sh

ENV DOCKER=true


ENTRYPOINT ["./ros_entrypoint.sh"]