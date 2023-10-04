FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/ui/planner ros2_ws/src/ui/planner/
COPY ros2_ws/src/interfaces/ui_interfaces ros2_ws/src/interfaces/ui_interfaces/
COPY ros2_ws/src/interfaces/neuronavigation_interfaces ros2_ws/src/interfaces/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && \
    colcon build --symlink-install

# Copy entrypoint

COPY ros2_ws/src/ui/planner/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
