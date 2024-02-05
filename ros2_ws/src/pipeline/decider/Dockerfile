FROM osrf/ros:iron-desktop

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

# HACK: Currently needed for mneflow to install successfully. Remove once the corresponding problem
#   in mneflow has been fixed.
ENV SKLEARN_ALLOW_DEPRECATED_SKLEARN_PACKAGE_INSTALL=True

RUN pip3 install git+https://github.com/zubara/mneflow.git@38dcee430

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace and build packages

WORKDIR /app/ros2_ws

# ROS interfaces
COPY ros2_ws/src/interfaces/eeg_interfaces src/interfaces/eeg_interfaces/
COPY ros2_ws/src/interfaces/event_interfaces src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/pipeline_interfaces src/interfaces/pipeline_interfaces/
COPY ros2_ws/src/interfaces/project_interfaces src/interfaces/project_interfaces/
COPY ros2_ws/src/interfaces/system_interfaces src/interfaces/system_interfaces/
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
