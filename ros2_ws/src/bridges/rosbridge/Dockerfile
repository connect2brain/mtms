FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    python3-pip

RUN pip install \
    pymongo \
    tornado

# Copy ROS workspace for rosbridge

WORKDIR /app/ros2_ws
COPY ros2_ws/src/bridges/rosbridge/rosbridge_suite src/bridges/rosbridge_suite
COPY ros2_ws/src/interfaces/eeg_interfaces src/interfaces/eeg_interfaces
COPY ros2_ws/src/interfaces/mtms_interfaces src/interfaces/mtms_interfaces
COPY ros2_ws/src/interfaces/event_interfaces src/interfaces/event_interfaces
COPY ros2_ws/src/interfaces/fpga_interfaces src/interfaces/fpga_interfaces
COPY ros2_ws/src/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces

COPY ros2_ws/src/bridges/rosbridge/ros_entrypoint.sh .
RUN chmod +x ros_entrypoint.sh

# Change line endings in all files in rosbridge

RUN shopt -s globstar && dos2unix **

# Build ROS packages

RUN source /opt/ros/galactic/setup.bash && \
    colcon build

# XXX: Hack around the symlink issue, allows fail with || : as this isn't needed on linux

RUN cp install/rosbridge_server/lib/rosbridge_server/rosbridge_websocket.py install/rosbridge_server/lib/rosbridge_server/rosbridge_websocket || :

# Define entrypoint

ENTRYPOINT ["./ros_entrypoint.sh"]
