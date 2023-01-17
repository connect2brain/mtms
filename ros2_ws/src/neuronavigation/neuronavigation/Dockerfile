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
    python3-vtkgdcm \
    python3-gdcm \
    cython3 \
    python3-h5py \
    python3-imageio \
    python3-keras \
    python3-pubsub \
    dos2unix \
    locales \
    python3-pip

RUN locale-gen  \
    en_US.UTF-8 \
    en_GB.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

RUN pip install vtk==9.1.0 pyacvd==0.2.7

## InVesalius

WORKDIR /app

# Copy and build Cython modules

COPY ros2_ws/src/neuronavigation/neuronavigation/invesalius3/invesalius_cy ros2_ws/src/neuronavigation/neuronavigation/invesalius3/invesalius_cy/
COPY ros2_ws/src/neuronavigation/neuronavigation/invesalius3/setup.py ros2_ws/src/neuronavigation/neuronavigation/invesalius3/
WORKDIR /app/ros2_ws/src/neuronavigation/neuronavigation/invesalius3
RUN python3 setup.py build_ext --inplace

# HACK: Delete setup.py after running to avoid confusing colcon, which searches for setup.py files.
#
RUN rm setup.py

# Copy and build ROS interfaces

WORKDIR /app
COPY ros2_ws/src/interfaces/neuronavigation_interfaces ros2_ws/src/interfaces/neuronavigation_interfaces/
COPY ros2_ws/src/interfaces/mtms_interfaces ros2_ws/src/interfaces/mtms_interfaces/

RUN cd ros2_ws && \
    source /opt/ros/galactic/setup.bash && \
    colcon build --packages-select neuronavigation_interfaces mtms_interfaces

# Copy rest of InVesalius

COPY ros2_ws/src/neuronavigation/neuronavigation/invesalius3 ros2_ws/src/neuronavigation/neuronavigation/invesalius3/
COPY ros2_ws/src/neuronavigation/neuronavigation/pypolaris-0.0.3-py3-none-any.whl ros2_ws/src/neuronavigation/neuronavigation/invesalius3/
RUN pip install /app/ros2_ws/src/neuronavigation/neuronavigation/invesalius3/pypolaris-0.0.3-py3-none-any.whl
# Copy and build Neuronavigation ROS package

COPY ros2_ws/src/neuronavigation/neuronavigation ros2_ws/src/neuronavigation/neuronavigation/

RUN cd ros2_ws && \
    source /opt/ros/galactic/setup.bash && \
    colcon build --packages-select neuronavigation

COPY ros2_ws/src/neuronavigation/neuronavigation/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
