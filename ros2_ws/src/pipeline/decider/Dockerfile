FROM osrf/ros:iron-desktop@sha256:eddad5f79aa7e888af29793c11f0ee44280c656b74dba0d37b2f48a0aa57352e

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    python3-pip \
    python3-tk \
    ros-iron-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Note: Tensorflow is needed for mneflow, which is used for machine learning.
RUN pip3 install tensorflow

RUN pip3 install mneflow

# Install statsmodels for the pipeline
RUN pip3 install statsmodels

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY ros2_ws/src/interfaces/eeg_interfaces src/interfaces/eeg_interfaces/
COPY ros2_ws/src/interfaces/event_interfaces src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/experiment_interfaces src/interfaces/experiment_interfaces/
COPY ros2_ws/src/interfaces/mep_interfaces src/interfaces/mep_interfaces/
COPY ros2_ws/src/interfaces/pipeline_interfaces src/interfaces/pipeline_interfaces/
COPY ros2_ws/src/interfaces/project_interfaces src/interfaces/project_interfaces/
COPY ros2_ws/src/interfaces/system_interfaces src/interfaces/system_interfaces/
COPY ros2_ws/src/interfaces/targeting_interfaces src/interfaces/targeting_interfaces/
RUN source /opt/ros/iron/setup.bash && colcon build

# ROS node
COPY ros2_ws/src/pipeline/decider src/pipeline/decider/
RUN source /opt/ros/iron/setup.bash && colcon build --symlink-install --packages-select decider

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/pipeline/decider/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
