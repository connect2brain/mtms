FROM osrf/ros:iron-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

RUN apt-get update

RUN apt-get install -y \
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp \
    wget

RUN wget https://github.com/Kitware/CMake/releases/download/v3.26.3/cmake-3.26.3-linux-x86_64.tar.gz

RUN tar xf cmake-3.26.3-linux-x86_64.tar.gz

WORKDIR /app/ros2_ws

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Build interfaces
COPY ros2_ws/src/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces/
RUN source /opt/ros/iron/setup.bash && colcon build


# Copy efield packages
COPY ros2_ws/src/targeting/efield src/targeting/efield/

# Required so efield and efield_library can find their dependencies
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/app/ros2_ws/src/targeting/efield/efield_libraries/lib"
ENV PATH="/cmake-3.26.3-linux-x86_64/bin:${PATH}"


# Build efield library
RUN cd src/targeting/efield/efield_libraries && cmake . && make

# Build efield packages
RUN source /opt/ros/iron/setup.bash && colcon build --packages-select efield


COPY ros2_ws/src/targeting/efield/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
