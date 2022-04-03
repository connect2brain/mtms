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
COPY ros2_ws/src/rosbridge/rosbridge_suite src/rosbridge_suite
COPY ros2_ws/src/mtms_interfaces src/mtms_interfaces
COPY ros2_ws/src/neuronavigation_interfaces src/neuronavigation_interfaces

COPY ros2_ws/src/rosbridge/ros_entrypoint.sh .
RUN chmod +x ros_entrypoint.sh

# Change line endings in all files in rosbridge

RUN shopt -s globstar && dos2unix **

# Build ROS packages

RUN source /opt/ros/galactic/setup.bash && \
    colcon build --symlink-install

# XXX: Hack around the symlink issue

RUN cp install/rosbridge_server/lib/rosbridge_server/rosbridge_websocket.py install/rosbridge_server/lib/rosbridge_server/rosbridge_websocket

# Define entrypoint

ENTRYPOINT ["./ros_entrypoint.sh"]
