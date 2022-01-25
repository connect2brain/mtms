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
    python3-pubsub

# Copy ROS workspace, including InVesalius

WORKDIR /app
COPY ros2_ws ros2_ws/

# Build Cython modules for InVesalius

WORKDIR /app/ros2_ws/src/neuronavigation_pkg/invesalius3
RUN python3 setup.py build_ext --inplace

# Build ROS packages

WORKDIR /app
RUN cd ros2_ws && \
    source /opt/ros/galactic/setup.bash && \
    colcon build

COPY ros_entrypoint.sh /
RUN chmod +x /ros_entrypoint.sh

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]