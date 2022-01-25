# ROS wrapper for InVesalius

## Cloning the repository

- Run:

```
git clone --recurse-submodules git@github.com:connect2brain/invesalius_ros.git
```

## Running without Docker

### Install ROS

- Install ROS by following the installation [instructions](https://docs.ros.org/en/galactic/Installation.html) from their website.

  Any installation option should work, but at least these instructions have been tested:

  https://docs.ros.org/en/galactic/Installation/Ubuntu-Install-Binary.html

### Install InVesalius

- Install the dependencies needed by InVesalius3 (see installation instructions in [Wiki](https://github.com/invesalius/invesalius3/wiki))

- Build the Cython modules needed by InVesalius3 (likewise, see installation instructions). Note that InVesalius
  resides in the directory `invesalius_ros/ros2_ws/src/neuronavigation_pkg/invesalius3`, therefore the modules
  need to be built in that directory.

- You can check that the installation succeeded by running `python3 app.py` in the above directory and checking
  that InVesalius starts.

### Run InVesalius

- In Windows, run the following commands:

```
cd ros2_ws
call c:\dev\ros2_galactic\local_setup.bat
colcon build
call install\local_setup.bat
ros2 run neuronavigation_pkg start
```

- In Linux, run the following commands:

```
cd ros2_ws
. ~/ros2_galactic/ros2-linux/setup.bash
colcon build
. install/local_setup.bash
ros2 run neuronavigation_pkg start
```

## Troubleshooting

### error: `colcon: command not found`

- Run `sudo apt install python3-colcon-common-extensions`

### While running colcon, error: `Could NOT find OpenSSL, try to set the path to OpenSSL root folder in the system variable OPENSSL_ROOT_DIR`

- Run `sudo apt-get install libssl-dev`

## Running with Docker

- Install Docker

- Run the following commands in `invesalius_ros` directory:

```
sudo docker -t neuronavigation .
xhost local:root
sudo docker run --rm -it --env DISPLAY --volume /tmp/.X11-unix:/tmp/.X11-unix:rw neuronavigation ros2 run neuronavigation_pkg start
```
