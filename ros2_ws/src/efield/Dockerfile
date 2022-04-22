FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    liblapacke-dev \
    liblapack-dev \
    liblapack3 \
    libopenblas-base \
    libopenblas-dev

# Copy ROS workspace for e-field

WORKDIR /app
COPY ros2_ws/src/efield ros2_ws/src/efield/
COPY ros2_ws/src/neuronavigation_interfaces ros2_ws/src/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app/ros2_ws

COPY ros2_ws/src/efield/efield_libraries/libs/linalg ros2_ws/src/efield/efield_libraries/libs/linalg/
COPY ros2_ws/src/efield/efield_libraries/libs/tmsutil ros2_ws/src/efield/efield_libraries/libs/tmsutil/
COPY ros2_ws/src/efield/efield_libraries/libs/phi ros2_ws/src/efield/efield_libraries/libs/phi/
COPY ros2_ws/src/efield/efield_libraries/libs/tms ros2_ws/src/efield/efield_libraries/libs/tms/
COPY ros2_ws/src/efield/efield_libraries/efield_estimation ros2_ws/src/efield/efield_libraries/efield_estimation/

RUN source /opt/ros/galactic/setup.bash && \
    colcon build

COPY ros2_ws/src/efield/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
