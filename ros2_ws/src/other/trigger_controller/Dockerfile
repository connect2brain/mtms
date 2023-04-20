FROM osrf/ros:humble-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    python3-pip

RUN pip install PySerial

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/other/trigger_controller ros2_ws/src/other/trigger_controller/
COPY ros2_ws/src/interfaces/neuronavigation_interfaces ros2_ws/src/interfaces/neuronavigation_interfaces/
COPY ros2_ws/src/interfaces/ui_interfaces ros2_ws/src/interfaces/ui_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/humble/setup.bash && \
    colcon build

# Copy entrypoint

COPY ros2_ws/src/other/trigger_controller/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
