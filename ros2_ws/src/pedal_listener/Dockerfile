FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    python3-pip

RUN pip install PySerial

# Copy ROS workspace for pedal listener

WORKDIR /app
COPY ros2_ws/src/pedal_listener ros2_ws/src/pedal_listener/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build

# Copy entrypoint

COPY ros2_ws/src/pedal_listener/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
