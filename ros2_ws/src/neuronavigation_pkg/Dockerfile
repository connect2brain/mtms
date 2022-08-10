FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies for InVesalius

RUN apt-get update

ENV DEBIAN_FRONTEND=noninteractive
#RUN apt-get install -y python3-pip
#RUN pip3 install https://www.vtk.org/files/release/9.1/vtk-9.1.0-cp38-cp38-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
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
    python3-pip \

RUN pip3 install pyacvd==0.2.7
RUN pip3 install pyvista

RUN locale-gen  \
    en_US.UTF-8 \
    en_GB.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

RUN pip install vtk==9.1.0

## InVesalius

WORKDIR /app

# Copy and build Cython modules

COPY ros2_ws/src/neuronavigation_pkg/invesalius3/invesalius_cy ros2_ws/src/neuronavigation_pkg/invesalius3/invesalius_cy/
COPY ros2_ws/src/neuronavigation_pkg/invesalius3/setup.py ros2_ws/src/neuronavigation_pkg/invesalius3/
WORKDIR /app/ros2_ws/src/neuronavigation_pkg/invesalius3
RUN python3 setup.py build_ext --inplace

# Copy rest of InVesalius

WORKDIR /app
COPY ros2_ws/src/neuronavigation_pkg/invesalius3 ros2_ws/src/neuronavigation_pkg/invesalius3/
RUN pip install /app/ros2_ws/src/neuronavigation_pkg/invesalius3/pypolaris-0.0.3-py3-none-any.whl
## Copy rest of the ROS workspace

WORKDIR /app
COPY ros2_ws/src/neuronavigation_pkg ros2_ws/src/neuronavigation_pkg/
COPY ros2_ws/src/neuronavigation_interfaces ros2_ws/src/neuronavigation_interfaces/
COPY ros2_ws/src/mtms_interfaces ros2_ws/src/mtms_interfaces/

# Build ROS packages

WORKDIR /app
RUN cd ros2_ws && \
    source /opt/ros/galactic/setup.bash && \
    colcon build

COPY ros2_ws/src/neuronavigation_pkg/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
