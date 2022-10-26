FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

# Copy ROS workspace for e-field

WORKDIR /app
COPY ros2_ws/src/targeting/efield ros2_ws/src/targeting/efield/
COPY ros2_ws/src/neuronavigation/neuronavigation_interfaces ros2_ws/src/neuronavigation/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && \
    colcon build

COPY ros2_ws/src/targeting/efield/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
