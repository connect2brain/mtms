FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update && \
    apt-get install -y \
    dos2unix \
    ros-jazzy-rmw-cyclonedds-cpp && \
    rm -rf /var/lib/apt/lists/*

# Set the Cyclone DDS configuration
COPY ros2_ws/src/shared_packages/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/mtms_packages/ui/planner ros2_ws/src/mtms_packages/ui/planner/
COPY ros2_ws/src/mtms_packages/interfaces/ui_interfaces ros2_ws/src/mtms_packages/interfaces/ui_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/neuronavigation_interfaces ros2_ws/src/mtms_packages/interfaces/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/jazzy/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/mtms_packages/ui/planner/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
