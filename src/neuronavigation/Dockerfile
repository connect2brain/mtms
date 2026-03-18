FROM docker.io/ros:jazzy@sha256:be7350a0efd17925415a6a3a076d6009b425af7500b0a5cdb5d717e87ef46f39

SHELL ["/bin/bash", "-c"]

# Install dependencies

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    # General
    dos2unix \
    ros-jazzy-rmw-cyclonedds-cpp \
    # InVesalius
    locales \
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
    python3-h5py \
    python3-imageio \
    python3-pubsub \
    python3-pip && \
    rm -rf /var/lib/apt/lists/*

# Install Python dependencies

RUN pip install cython vtk==9.2.6 plaidml-keras==0.7.0 pyacvd==0.2.9 pypolaris==0.0.6 python-socketio scikit-learn
RUN pip install numpy==1.26.4 scipy==1.14.0
# HACK: As of now, wxPython >=4.1.0 (required by InVesalius) for Ubuntu 22 is not stored in PyPi, hence
#       install using an own wheel file.
#
COPY src/neuronavigation/wheels/wxPython-4.2.0-cp310-cp310-linux_x86_64.whl /app/wheels/

RUN pip install /app/wheels/wxPython-4.2.0-cp310-cp310-linux_x86_64.whl

# Language and locale
RUN locale-gen  \
    en_US.UTF-8 \
    en_GB.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

## InVesalius

WORKDIR /app

# Copy and build Cython modules

COPY src/neuronavigation/invesalius3/invesalius_cy src/neuronavigation/invesalius3/invesalius_cy/
COPY src/neuronavigation/invesalius3/setup.py src/neuronavigation/invesalius3/
WORKDIR /app/src/neuronavigation/invesalius3
RUN python3 setup.py build_ext --inplace

# HACK: Delete setup.py after running to avoid confusing colcon, which searches for setup.py files.
#
RUN rm setup.py

# Copy and build ROS interfaces

WORKDIR /app

COPY src/interfaces/event_interfaces src/interfaces/event_interfaces/
COPY src/interfaces/experiment_interfaces src/interfaces/experiment_interfaces/
COPY src/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces/
COPY src/interfaces/targeting_interfaces src/interfaces/targeting_interfaces/
COPY src/interfaces/ui_interfaces src/interfaces/ui_interfaces/
COPY src/interfaces/waveform_interfaces src/interfaces/waveform_interfaces/

COPY src/interfaces/eeg_interfaces src/interfaces/eeg_interfaces/
COPY src/interfaces/mep_interfaces src/interfaces/mep_interfaces/
COPY src/interfaces/mtms_trial_interfaces src/interfaces/mtms_trial_interfaces/

RUN source /opt/ros/jazzy/setup.bash && colcon build

# Copy rest of InVesalius

COPY src/neuronavigation/invesalius3 src/neuronavigation/invesalius3/

# Ensure an empty __init__.py exists for Python package discovery in invesalius3

RUN set -eux; \
    pkg_dir="/app/src/neuronavigation/invesalius3"; \
    init_file="${pkg_dir}/__init__.py"; \
    mkdir -p "${pkg_dir}"; \
    : > "${init_file}"

# Copy and build Neuronavigation ROS package

COPY src/neuronavigation src/neuronavigation/

RUN source /opt/ros/jazzy/setup.bash && colcon build --packages-select neuronavigation

# Copy entrypoint

WORKDIR /app

COPY src/neuronavigation/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
