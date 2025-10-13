FROM osrf/ros:iron-desktop@sha256:eddad5f79aa7e888af29793c11f0ee44280c656b74dba0d37b2f48a0aa57352e

SHELL ["/bin/bash", "-c"]

# Pin to the latest stable version of ros-apt-source
ENV ROS_APT_SOURCE_VERSION=1.1.0
# Remove existing ROS 2 APT sources and keyrings to prevent conflicts
RUN rm -f /etc/apt/sources.list.d/ros2-latest.list && \
    rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg
# Install curl and certificates required for downloading files securely
RUN apt-get update && apt-get install -y curl ca-certificates
# Use ROS_APT_SOURCE_VERSION and OS codename to build download URL and install the .deb
RUN . /etc/os-release && \
    curl -L -s -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${VERSION_CODENAME}_all.deb" && \
    apt-get update && \
    apt-get install -y /tmp/ros2-apt-source.deb && \
    rm -f /tmp/ros2-apt-source.deb

# Install dependencies

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    # General
    dos2unix \
    ros-iron-rmw-cyclonedds-cpp \
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
COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/wheels/wxPython-4.2.0-cp310-cp310-linux_x86_64.whl /app/wheels/

RUN pip install /app/wheels/wxPython-4.2.0-cp310-cp310-linux_x86_64.whl

# Language and locale
RUN locale-gen  \
    en_US.UTF-8 \
    en_GB.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Set the Cyclone DDS configuration
COPY ros2_ws/src/shared_packages/resources/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

## InVesalius

WORKDIR /app

# Copy and build Cython modules

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3/invesalius_cy ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3/invesalius_cy/
COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3/setup.py ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3/
WORKDIR /app/ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3
RUN python3 setup.py build_ext --inplace

# HACK: Delete setup.py after running to avoid confusing colcon, which searches for setup.py files.
#
RUN rm setup.py

# Copy and build ROS interfaces

WORKDIR /app/ros2_ws

COPY ros2_ws/src/mtms_packages/interfaces/event_msgs src/interfaces/event_msgs/
COPY ros2_ws/src/mtms_packages/interfaces/experiment_interfaces src/interfaces/experiment_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/neuronavigation_interfaces src/interfaces/neuronavigation_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/targeting_services src/interfaces/targeting_services/
COPY ros2_ws/src/mtms_packages/interfaces/ui_interfaces src/interfaces/ui_interfaces/
COPY ros2_ws/src/mtms_packages/interfaces/waveform_msgs src/interfaces/waveform_msgs/

COPY ros2_ws/src/shared_packages/interfaces/eeg_msgs src/shared_interfaces/eeg_msgs/
COPY ros2_ws/src/shared_packages/interfaces/mep_interfaces src/shared_interfaces/mep_interfaces/
COPY ros2_ws/src/shared_packages/interfaces/mtms_trial_interfaces src/shared_interfaces/mtms_trial_interfaces/
COPY ros2_ws/src/shared_packages/interfaces/targeting_msgs src/shared_interfaces/targeting_msgs/

RUN source /opt/ros/iron/setup.bash && colcon build

# Copy rest of InVesalius

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3 src/neuronavigation/neuronavigation/invesalius3/

# Ensure an empty __init__.py exists for Python package discovery in invesalius3

RUN set -eux; \
    pkg_dir="/app/ros2_ws/src/neuronavigation/neuronavigation/invesalius3"; \
    init_file="${pkg_dir}/__init__.py"; \
    mkdir -p "${pkg_dir}"; \
    : > "${init_file}"

# Copy and build Neuronavigation ROS package

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation src/neuronavigation/neuronavigation/

RUN source /opt/ros/iron/setup.bash && colcon build --packages-select neuronavigation

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
