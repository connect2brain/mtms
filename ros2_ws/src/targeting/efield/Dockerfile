FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix

WORKDIR /app/ros2_ws


# Build interfaces
COPY ros2_ws/src/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces/
RUN source /opt/ros/galactic/setup.bash && colcon build


# Copy efield packages
COPY ros2_ws/src/targeting/efield src/targeting/efield/

# Required so efield and efield_library can find their dependencies
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/app/ros2_ws/src/targeting/efield/efield_libraries/lib"

# Build efield library
RUN cd src/targeting/efield/efield_libraries && cmake . && make

# Build efield packages
RUN source /opt/ros/galactic/setup.bash && colcon build --packages-select efield


COPY ros2_ws/src/targeting/efield/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
