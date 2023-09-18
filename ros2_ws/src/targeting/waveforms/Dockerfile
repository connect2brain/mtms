FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies
WORKDIR /build

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/targeting/waveforms ros2_ws/src/targeting/waveforms/
COPY ros2_ws/src/interfaces/event_interfaces ros2_ws/src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/targeting_interfaces ros2_ws/src/interfaces/targeting_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/iron/setup.bash && \
    colcon build

# Copy entrypoint

COPY ros2_ws/src/targeting/waveforms/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
