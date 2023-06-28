# mTMS project

## Copyright and permitted use

All rights to the code in this repository (excluding the submodules) belongs to Aalto University.

Currently, its permitted use is limited to people in Connect2Brain project in Aalto University, the Eberhard Karl University of Tübingen,
and D'Annunzio University of Chieti–Pescara, and Aalto University students and employees performing research assignments in Aalto C2B laboratory
or developing the software under a non-disclosure agreement.

The software also uses external software and libraries, which reside in their own repositories, are connected to this
repository using Git submodules, and have their own copyright owners. Here is the list of the external repositories
and where they are located in the directory structure:

[InVesalius3](https://github.com/invesalius/invesalius3), `ros2_ws/src/neuronavigation/neuronavigation/invesalius3`

[rosbridge_suite](https://github.com/RobotWebTools/rosbridge_suite/), `ros2_ws/src/bridges/rosbridge_suite`

e-field library, `ros2_ws/src/targeting/efield/src`

Please see the repository roots of the external repositories for their respective authors and licenses.

## Installation

- Install Ubuntu 22.04

- Install Git by running:

```
sudo apt install git
```

- Install Git Large File Storage (Git LFS):

```
sudo apt install git-lfs
```

- Install Git Credential Manager (GCM):

```
wget "https://github.com/GitCredentialManager/git-credential-manager/releases/download/v2.0.935/gcm-linux_amd64.2.0.935.deb" -O /tmp/gcmcore.deb
sudo dpkg -i /tmp/gcmcore.deb
git-credential-manager configure
git config --global credential.credentialStore secretservice
```

- Clone the repository:

```
git clone --recurse-submodules git@github.com:connect2brain/mtms.git
```

When asked, select the option: "Sign in with your browser" and enter your GitHub username and password.

### Installation (continued; for personal computer)

- Install Docker:

```
sudo apt install docker.io
sudo groupadd docker
sudo usermod -aG docker $USER
sudo apt install docker-compose
```

- Install C++ tools (for real-time pipeline):

```
sudo apt install cmake
sudo apt install g++
```

- Install ROS by running the following in the repository root:

```
cd scripts/linux/installation
sudo install-ros
```

### Installation (continued; for lab computer)

- Install real-time kernel patch via Ubuntu Pro live patching.

- Run the following in the repository root:

```
cd scripts/linux/installation
sudo setup-lab-computer
```

- Install drivers for the FPGA. See the instructions: https://www.ni.com/fi-fi/support/documentation/supplemental/18/downloading-and-installing-ni-driver-software-on-linux-desktop.html.
On step 4, install packages `ni-rseries` and `ni-fpga-interface`.

- Configure network to allow receiving data from the EEG device. Go to: Settings -> Network -> Wired options -> IPv4. Set IPv4 Method to "Manual". Set address
to 192.168.200.221 and netmask to 255.255.255.0, and apply changes.

- Modify `/etc/security/limits.conf` and add the following lines:

```
<your username>    -   rtprio    98
<your username>    -   memlock   2000000
```

- Install MATLAB with the following toolboxes: ROS toolbox, Statistics and machine learning toolbox.

- Open MATLAB by running:

```
sudo matlab
```

This ensures that you can change all configuration parameters, such as the path.

- Open HOME -> Set Path -> Add Folder. Add the folder `/home/mtms/mtms/api/matlab`.
Open Keyboard -> Shortcuts and change the value of "Active settings" to "Windows Default Set".
Apply changes.

- (Optional) Modify keybinding for Ctrl-Alt-T to open the new terminal in top-left corner of the
screen:

Go to Settings -> Keyboard -> Keyboard Shortcuts -> View and Customize Shortcuts. Select Custom Shortcuts and
press +. Set name to "Open Terminal Top Left", and set shortcut to Ctrl-Alt-T.
Set command to:

```
bash -c 'gnome-terminal & sleep 0.3s && wmctrl -r :ACTIVE: -e 0,0,0,-1,-1'
```

Press "Replace".

# Real-time pipeline

## Getting started

### Pipeline with EEG preprocessor

- To start the pipeline using only EEG preprocessor, run:

```
docker-compose up eeg_preprocessor
```

- Modify `.env` file in repository root to change the configuration of the EEG preprocessor:

* `EEG_PREPROCESSOR_TYPE`: One of "python", "compiledmatlab", or "cpp".
* `EEG_PREPROCESSOR_SCRIPT`: Can be used to select custom EEG preprocessors, stored in `pipeline/python/eeg_preprocessors/`
directory in repository root.

- The changes in `.env` file take place after restarting Docker container by running:

```
docker-compose restart eeg_preprocessor
```

### Simulated EEG data

- To stream simulated EEG data, run:

```
docker-compose up eeg_simulator
```

- Modify `.env` file in repository root to stream custom data in CSV format:

* `EEG_SIMULATOR_DATA_FILE_NAME`: The file from which the simulated EEG data is read from, stored in `data/eeg/` directory in repository root.
* `EEG_SIMULATOR_SAMPLING_FREQUENCY`: The sampling frequency used for streaming.
* `EEG_SIMULATOR_NUMBER_OF_EEG_CHANNELS`: The number of streamed EEG channels, read as the first N columns of the CSV file.
* `EEG_SIMULATOR_NUMBER_OF_EMG_CHANNELS`: The number of streamed EMG channels, read as the next M columns of the CSV file.

- The changes in `.env` file take place after restarting Docker container.

### Recording bags

- To record ROS bags while the pipeline is running, run:

```
docker-compose up bag_recorder
```

- Modify `.env` file in repository root to change the configuration:

* `BAG_NAME`: The bag name for recording the data, e.g., "experiment1".

- The changes in `.env` file take place after restarting Docker container.

### Exporting bags

- To export ROS bags to CSV, run:

```
docker-compose up bag_exporter
```

- Modify `.env` file in repository root to change the configuration:

* `BAG_ANALYSIS_BAG_NAME`: The bag name for exporting the CSV.
* `BAG_ANALYSIS_TIMESTAMP`: Optional. If not given, use the latest bag with the given name for exporting. An example value: `2023-04-26_11-26-52`.
* `BAG_ANALYSIS_TOPIC`: The topic to export, e.g., `/eeg/cleaned_data`.

- The changes in `.env` file take place after restarting Docker container.

# Documentation

## Build documentation locally

The documentation source files are located in `docs` and are built using [Sphinx](https://www.sphinx-doc.org/en/master/usage/quickstart.html). 

To build the documentation locally, first make sure your environment has completed the installation steps described [above](#installation). Particularly, make sure you have ROS activated: e.g. in Ubuntu 22.04/bash, use `source /opt/ros/humble/setup.bash`.

Subsequently, install the Sphinx dependencies
```
python3 -m pip install --r docs/sphinx-requirements.txt
```

Finally, enter the `docs` folder and build the documentation
```
cd docs
make clean
make html
```

The documentation can then be found in `docs/build/html/` folder and can be viewed with your favorite web browser. A natural place to start is `docs/build/html/index.html`.



# Legacy documentation

## Running without Docker

### Install ROS

- Install ROS by following the installation [instructions](https://docs.ros.org/en/galactic/Installation.html) from their website.

  Any installation option should work, but at least these instructions have been tested:

  https://docs.ros.org/en/galactic/Installation/Ubuntu-Install-Binary.html

### Install InVesalius

- Install the dependencies needed by InVesalius3 (see installation instructions in [Wiki](https://github.com/invesalius/invesalius3/wiki))

- Build the Cython modules needed by InVesalius3 (likewise, see installation instructions). Note that InVesalius
  resides in the directory `invesalius_ros/ros2_ws/src/neuronavigation/neuronavigation/invesalius3`, therefore the modules
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
ros2 run neuronavigation start
```

- In Linux, run the following commands:

```
cd ros2_ws
. ~/ros2_galactic/ros2-linux/setup.bash
colcon build
. install/local_setup.bash
ros2 run neuronavigation start
```

## Troubleshooting

### error: `colcon: command not found`

- Run `sudo apt install python3-colcon-common-extensions`

### While running colcon, error: `Could NOT find OpenSSL, try to set the path to OpenSSL root folder in the system variable OPENSSL_ROOT_DIR`

- Run `sudo apt-get install libssl-dev`

## Running with Docker

### Linux

- Install Docker and Docker compose
  Note: do not install docker with snap, will cause problems with X Display

- Git Large File Storage needs to be installed for the project to work properly.

- Check your $DISPLAY variable for example with `echo $DISPLAY`. Update .env file DISPLAY to that. Most likely it's enough to set it just as `:0`.

- Run the following commands on the root directory of the repository

```
xhost local:root
docker-compose up
```

### WSL

- Install XMing on Windows

- Run the rest of the commands in WSL

- Install Docker and Docker compose

- Check your `$DISPLAY` variable for example with `echo $DISPLAY`. Update .env file DISPLAY to that. Most likely it's enough to set it just as `:0` or `:0.0`.

- Run the following commands on the root directory of the repository

```
docker-compose up
```

#### Troubleshooting

These two links should be useful for troubleshooting:
1) https://github.com/microsoft/WSL/issues/6430
2) https://stackoverflow.com/questions/61860208/wsl-2-run-graphical-linux-desktop-applications-from-windows-10-bash-shell-erro

### Windows

#### Setup (done only once)

##### Setting up environment variables

- Add MTMS_PATH variable to system environment variables, pointing to the directory into which mTMS repository is cloned. This enables
using scripts stored in `scripts` directory to start the system.

##### Setting up Docker with X11 forwarding

- Install Docker

- Install X server, e.g., VcXsrv (using Choco: `choco install vcxsrv`). The following instructions are for VcXsrv.

- Start XLaunch. Tick the checkbox "Disable access control".

- Alternatively, run `C:\Program Files\VcXsrv\vcxsrv.exe" :0 -multiwindow -clipboard -wgl -ac`. This command can also be added to
Windows start-up.

- Run `ipconfig`, replace the IP address in DISPLAY variable in `.env` file with the host IP address reported by `ipconfig`, under the
section `Ethernet adapter vEthernet (WSL)`, and add `:0.0` to the end of the IP address, e.g., `DISPLAY=172.31.32.1:0.0`.

- Alternatively, run `python update_ip_for_wsl.py` in `scripts` directory, which updates DISPLAY variable in `.env` file automatically.

##### Setting up the pedal

- Install [Ubuntu 20.04 LTS](https://ubuntu.com/tutorials/install-ubuntu-on-wsl2-on-windows-10#1-overview) on WSL.

- Run `wsl --list`, it should look something like this:

```
docker-desktop (Default)
Ubuntu-20.04
```

- Set `Ubuntu-20.04` as the default distribution by running `wsl --set-default Ubuntu-20.04`.

- Follow the instructions on Microsoft [blog post](https://devblogs.microsoft.com/commandline/connecting-usb-devices-to-wsl/) under `Setup` heading for installing USB/IP. Do not proceed to `Attaching the device` heading.

#### Running the system

- Connect the pedal to a USB port.

- Open command prompt in administrator mode. Run `usbipd wsl list`, it should print something like this:

```
BUSID  DEVICE                                                        STATE
1-2    USB Input Device                                              Not attached
1-10   USB Serial Device (COM7)                                      Not attached
1-11   USB Input Device                                              Not attached
```

- Run `usbipd wsl attach --busid 1-10` (or similar if you have different bus id for the serial device).

- Alternatively, run `python initialize_usb_over_ip.py` in `scripts` directory, which automatically does the above for USB pedal and Polaris tracker.

- Run `docker-compose up -d` in `invesalius_ros` directory.


---

# Building
You can build the project docker images with `docker-compose.build.yml` by running `docker-compose -f docker-compose.build.yml build`. 

Use `tag-docker-images.sh` and `push-docker-tags.sh` to tag the built images and push them to Docker Hub.

Deploy new images workflow:
1. `docker-compose -f docker-compose.build.yml build`
2. `./tag-docker-images.sh`
3. `./push-docker-tags.sh`

---

# Production environment
Production environment utilizes Docker Swarm to deploy and manage nodes across two host machines. Ensure that the machines are connected to the same router.

The Docker Swarm configuration utilizes [weavenet](https://github.com/weaveworks/weave) to enable multicast across docker nodes. As stated in its documentation: "Weave Net creates a virtual network that connects Docker containers across multiple hosts and enables their automatic discovery."

Some services utilize Docker in Docker (DiD) as Docker Swarm does not natively support privileged containers. DiD is a way to bypass that.

### Terminology
- Manager: the host that runs the docker swarm and manages the services and workers
- Worker: a host that is controlled by the manager
- Service: a docker container that is running on some host. Our services all contain a single ros2 node

Note that node means different things in ros2 and docker terminology:  
Node in docker swarm = a host machine  
Node in ros2 = a ros2 node


### First time set up
1. If you have Windows hosts, install Docker Desktop and X Server (instructions in Docker -> Window section)
2. On all machines, install docker network adapter weavenet `docker plugin install weaveworks/net-plugin:latest_release`.
3. On the manager machine, log in to Dockerhub with an account that has access to our images `docker login`

### Running
If you have Windows hosts, start Docker Desktop before running any of these commands.

#### Neuronavigation config
On the host that neuronavigation will be used, do the following:

##### Windows:
- Start X Server (as instructed in the Docker -> Windows section)
- Check WSL2 ip with ipconfig. Set docker-compose.prod.yml neuronavigation DISPLAY to `<ip>:0.0`
Configure docker-compose.prod.yml neuronavigation DISPLAY variable to be the correct for the target host

##### Linux:
- Run `xhost local:root`
- If using docker swarm, set neuronavigation DISPLAY to `:0` or `:0.0` in `docker-stack.yml` file. If using docker-compose,
check that DISPLAY is set to `:0` or `:0.0` in `.env`.


After the previous steps are done, run the following commands to start the swarm cluster.
1. Host: Create docker swarm `docker swarm init`
2. Worker: Copy the docker swarm join command (`docker swarm join --token <token> <ip:port>`) produced by the previous command and run it on the worker  
3. Host: (Optional) Ensure that you can see both nodes in the network `docker node ls`
4. Host: Deploy docker stack `docker stack deploy -c docker-stack.yml mtms --with-registry-auth`
5. Host: (Optional) Check service status `docker service ls` and `docker service ps --no-trunc <service(s)>`

Useful commands:
- See simple information of all services: `docker service ls`
- See more information of all services: `docker service ps mtms_eeg_bridge_wrapper mtms_eeg_batcher_wrapper mtms_eeg_simulator mtms_neuronavigation mtms_efield mtms_rosbridge mtms_pulse_sequence_controller mtms_planner mtms_front mtms_mtms_device_bridge_wrapper mtms_eeg_processor_wrapper mtms_eeg_processor_wrapper --no-trunc`
- Remove all services `docker service rm mtms_eeg_batcher_wrapper mtms_eeg_processor_wrapper mtms_eeg_simulator mtms_efield mtms_mtms_device_bridge_wrapper mtms_front mtms_neuronavigation mtms_planner mtms_pulse_sequence_controller mtms_rosbridge`
- Delete all dangling docker containers `docker kill $(docker ps -q)`

### Notes
Docker in docker containers are left dangling even if the services are removed so at the moment they need to be manually removed.

# Real-time support

## Permissions
You need have permissions to set task priority and scheduling policy. Modify `/etc/security/limits.conf` and add the following lines:
```
<your username>    -   rtprio    98
<your username>    -   memlock   <limit in kB>

```

Realtime capabilities do not work on Windows.
