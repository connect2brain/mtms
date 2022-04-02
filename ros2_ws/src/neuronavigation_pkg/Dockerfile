FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies for InVesalius

RUN apt-get update

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get install -y \
    python3-wxgtk4.0 \
    python3-numpy \
    python3-scipy \
    python3-pil \
    python3-matplotlib \
    python3-skimage \
    python3-nibabel \
    python3-serial \
    python3-psutil \
    python3-vtk7 \
    python3-vtkgdcm \
    python3-gdcm \
    cython3 \
    python3-h5py \
    python3-imageio \
    python3-keras \
    python3-pubsub \
    dos2unix

# Copy InVesalius, build Cython modules

WORKDIR /app
COPY ros2_ws/src/neuronavigation_pkg/invesalius3 ros2_ws/src/neuronavigation_pkg/invesalius3/
WORKDIR /app/ros2_ws/src/neuronavigation_pkg/invesalius3
RUN python3 setup.py build_ext --inplace

# Copy rest of the ROS workspace

WORKDIR /app
COPY ros2_ws/src/neuronavigation_pkg ros2_ws/src/neuronavigation_pkg/
COPY ros2_ws/src/neuronavigation_interfaces ros2_ws/src/neuronavigation_interfaces/

# Build ROS packages

WORKDIR /app
RUN cd ros2_ws && \
    source /opt/ros/galactic/setup.bash && \
    colcon build

COPY ros2_ws/src/neuronavigation_pkg/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
