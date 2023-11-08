FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Copy ROS workspace

WORKDIR /app/ros2_ws
COPY ros2_ws/src/interfaces src/interfaces/

# Build ROS packages

RUN source /opt/ros/iron/setup.bash && colcon build

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/other/bag_recorder/bag_recorder.sh .
RUN dos2unix bag_recorder.sh
RUN chmod +x bag_recorder.sh

ENTRYPOINT ["./bag_recorder.sh"]
