FROM osrf/ros:iron-desktop@sha256:eddad5f79aa7e888af29793c11f0ee44280c656b74dba0d37b2f48a0aa57352e

SHELL ["/bin/bash", "-c"]

# Pin to the latest stable version of ros-apt-source
ENV ROS_APT_SOURCE_VERSION=1.1.0
ENV ROS_DISTRO_CODENAME=jammy
ENV ROS_APT_DEB_NAME=ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${ROS_DISTRO_CODENAME}_all.deb
ENV ROS_APT_DEB_URL=https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/${ROS_APT_DEB_NAME}
# Safely remove existing ROS 2 APT sources and keys (if present)
RUN rm -f /etc/apt/sources.list.d/ros2-latest.list
RUN rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg
# Install essential tools for downloading and verifying packages
RUN apt-get update && apt-get install -y curl ca-certificates gnupg
# Download the pinned version of the ros2-apt-source .deb
RUN curl -sL -o /tmp/${ROS_APT_DEB_NAME} ${ROS_APT_DEB_URL}
# Verify the DEB's SHA256 checksum
RUN echo "1600cb8cc28258a39bffc1736a75bcbf52d1f2db371a4d020c1b187d2a5a083b  /tmp/${ROS_APT_DEB_NAME}" | sha256sum -c -
# Install the DEB via apt, update package list, and clean up
RUN apt-get update \
    && apt-get install -y /tmp/${ROS_APT_DEB_NAME} \
    && rm -f /tmp/${ROS_APT_DEB_NAME} \

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

COPY ros2_ws/src/shared_packages/interfaces/mep_interfaces src/shared_interfaces/mep_interfaces/

RUN source /opt/ros/iron/setup.bash && colcon build --base-paths src/interfaces

# Copy rest of InVesalius

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3 src/neuronavigation/neuronavigation/invesalius3/

# Copy and build Neuronavigation ROS package

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation src/neuronavigation/neuronavigation/

RUN source /opt/ros/iron/setup.bash && colcon build --packages-select neuronavigation

# Copy entrypoint

WORKDIR /app

COPY ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
