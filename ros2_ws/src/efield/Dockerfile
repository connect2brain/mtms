FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies for InVesalius

RUN apt-get update

# Copy ROS workspace, including InVesalius

WORKDIR /app
COPY ros2_ws/src/efield ros2_ws/src/efield/
COPY ros2_ws/src/neuronavigation_interfaces ros2_ws/src/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build

COPY ros2_ws/src/efield/ros_entrypoint.sh /
RUN chmod +x /ros_entrypoint.sh

ENTRYPOINT ["/ros_entrypoint.sh"]
