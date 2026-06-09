FROM osrf/ros:jazzy-desktop

SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    ros-jazzy-rmw-cyclonedds-cpp \
    dos2unix \
    locales \
    curl \
    pkg-config \
    build-essential \
    python3-dev \
    python3-pip \
    python3-wxgtk4.0 \
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
    python3-numpy \
    python3-scipy \
    python3-vtk9 \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --break-system-packages \
    cython==3.0.10 \
    pyacvd==0.2.11 \
    pypolaris==0.0.6 \
    python-socketio \
    scikit-learn
RUN python3 -m pip install --break-system-packages torch torchvision torchaudio

# Optional tractography support. Place a Python-version-compatible Trekker wheel
# in src/neuronavigation/wheels/ to enable it; otherwise the image builds without
# Trekker and the tractography UI remains disabled.
COPY src/neuronavigation/wheels /tmp/neuronavigation-wheels
RUN if compgen -G "/tmp/neuronavigation-wheels/Trekker-*.whl" > /dev/null; then \
      python3 -m pip install --break-system-packages /tmp/neuronavigation-wheels/Trekker-*.whl || \
      echo "No compatible Trekker wheel found; building neuronavigation without tractography support"; \
    else \
      echo "No Trekker wheel found; building neuronavigation without tractography support"; \
    fi

RUN locale-gen en_US.UTF-8 en_GB.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8




WORKDIR /app


# Copy and build ROS interfaces
WORKDIR /app
COPY interfaces interfaces/
RUN source /opt/ros/jazzy/setup.bash && \
    colcon build --base-paths interfaces
COPY interfaces/mtms_waveform_interfaces interfaces/mtms_waveform_interfaces/
COPY interfaces/mtms_targeting_interfaces interfaces/mtms_targeting_interfaces/
COPY interfaces/mtms_event_interfaces interfaces/mtms_event_interfaces/
COPY interfaces/mtms_neuronavigation_interfaces interfaces/mtms_neuronavigation_interfaces/
#COPY interfaces/ui_interfaces src/interfaces/ui_interfaces/

RUN source /opt/ros/jazzy/setup.bash && \
    colcon build --base-paths interfaces

# Copy rest of InVesalius
COPY src/neuronavigation/invesalius3 src/neuronavigation/invesalius3/

# Build invesalius_rs native extension
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y && \
    source /root/.cargo/env && \
    cargo --version
ENV PATH="/root/.cargo/bin:$PATH"

RUN python3 -m pip install --break-system-packages maturin

RUN source /root/.cargo/env && \
    cd /app/src/neuronavigation/invesalius3/invesalius_rs && \
    maturin build --release
RUN python3 -m pip install --break-system-packages /app/src/neuronavigation/invesalius3/invesalius_rs/target/wheels/*.whl
RUN mkdir -p /app/src/neuronavigation/invesalius3/invesalius_rs && \
    NATIVE_SO=$(find / -name "_native*.so" 2>/dev/null | head -n 1) && \
    echo "Found native module: $NATIVE_SO" && \
    test -n "$NATIVE_SO" && \
    cp "$NATIVE_SO" /app/src/neuronavigation/invesalius3/invesalius_rs/ && \
    ls -l /app/src/neuronavigation/invesalius3/invesalius_rs/_native*.so
RUN PYTHONPATH=/app/src/neuronavigation/invesalius3 python3 -c "import invesalius_rs; from invesalius_rs import _native; print(_native)"
# Copy and build Neuronavigation ROS package
COPY src/neuronavigation src/neuronavigation/

RUN source /opt/ros/jazzy/setup.bash && \
    source /app/install/setup.bash && \
    colcon build --base-paths src/neuronavigation --packages-select neuronavigation
RUN mkdir -p /app/install/neuronavigation/lib/python3.12/site-packages/invesalius_rs && \
    cp /usr/local/lib/python3.12/dist-packages/_native/_native*.so \
       /app/install/neuronavigation/lib/python3.12/site-packages/invesalius_rs/ && \
    ls -l /app/install/neuronavigation/lib/python3.12/site-packages/invesalius_rs/_native*.so    
    
# Copy InVesalius translation files for Python 3.12 runtime
RUN mkdir -p /app/install/neuronavigation/lib/python3.12/site-packages/locale && \
    cp -r /app/src/neuronavigation/invesalius3/locale/* \
          /app/install/neuronavigation/lib/python3.12/site-packages/locale/

# Copy InVesalius icons to the path expected by invesalius.inv_paths.ICON_DIR
RUN mkdir -p /app/install/neuronavigation/lib/icons && \
    cp -r /app/src/neuronavigation/invesalius3/icons/* \
          /app/install/neuronavigation/lib/icons/ && \
    ls -l /app/install/neuronavigation/lib/icons/splash_en.png    

# Copy navigation assets to the Python 3.12 site-packages path expected by
# invesalius.inv_paths.OBJ_DIR.
RUN mkdir -p /app/install/neuronavigation/lib/python3.12/site-packages/navigation && \
    cp -r /app/src/neuronavigation/invesalius3/navigation/* \
          /app/install/neuronavigation/lib/python3.12/site-packages/navigation/ && \
    ls -l /app/install/neuronavigation/lib/python3.12/site-packages/navigation/objects/magstim_fig8_coil.stl
    
RUN sed -i 's/vtkIdFilter/vtkGenerateIds/g' \
    /app/install/neuronavigation/lib/python3.12/site-packages/invesalius/data/polydata_utils.py    
COPY src/neuronavigation/ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh

ENTRYPOINT ["./ros_entrypoint.sh"]
