FROM osrf/ros:galactic-desktop

SHELL ["/bin/bash", "-c"]

# Install dependencies

WORKDIR /build

RUN apt-get update

RUN apt-get install -y dos2unix wget

RUN wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN chmod +x Miniconda3-latest-Linux-x86_64.sh
RUN ./Miniconda3-latest-Linux-x86_64.sh -b

COPY ros2_ws/src/eeg/eeg_processor/environment.yml build/

CMD ["tail", "-f", "/dev/null"]

RUN /root/miniconda3/bin/conda env create --name default --file=build/environment.yml


# Copy ROS workspace

WORKDIR /app
COPY ros2_ws/src/eeg/eeg_processor ros2_ws/src/eeg/eeg_processor/
COPY ros2_ws/src/interfaces/event_interfaces ros2_ws/src/interfaces/event_interfaces/
COPY ros2_ws/src/interfaces/mtms_device_interfaces ros2_ws/src/interfaces/mtms_device_interfaces/
COPY ros2_ws/src/interfaces/eeg_interfaces ros2_ws/src/interfaces/eeg_interfaces/



# Build ROS packages

WORKDIR /app/ros2_ws
RUN source /opt/ros/galactic/setup.bash && colcon build

# Do this after colcon build as otherwise colcon gets confused about separate python installations
SHELL ["/root/miniconda3/bin/conda", "run", "-n", "default", "/bin/bash", "-c"]

RUN pip install mne scipy


# Create a directory for hotswappable processors

RUN mkdir -p hotswappable_processors
RUN mkdir -p measurements

# Copy entrypoint

COPY ros2_ws/src/eeg/eeg_processor/ros_entrypoint.sh .
COPY ros2_ws/src/eeg/eeg_processor/preprocessor_ros_entrypoint.sh .
COPY ros2_ws/src/eeg/eeg_processor/stimulus_presenter_ros_entrypoint.sh .
RUN dos2unix ros_entrypoint.sh stimulus_presenter_ros_entrypoint.sh preprocessor_ros_entrypoint.sh
RUN chmod +x ros_entrypoint.sh stimulus_presenter_ros_entrypoint.sh preprocessor_ros_entrypoint.sh

ENV DOCKER=true

ENTRYPOINT ["./ros_entrypoint.sh"]