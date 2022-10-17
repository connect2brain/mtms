# ROS wrapper for InVesalius

## Cloning the repository

- Run:

```
git clone --recurse-submodules git@github.com:connect2brain/mtms.git
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

### Linux

- Install Docker and Docker compose

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

##### Setting up Docker with X11 forwarding

- Install Docker

- Install X server, e.g., VcXsrv (using Choco: `choco install vcxsrv`). The following instructions are for VcXsrv.

- Start XLaunch. Tick the checkbox "Disable access control".

- Run `ipconfig`, replace the IP address in DISPLAY variable in `.env` file with the host IP address reported by `ipconfig`.

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

- Run `docker-compose up -d` in `invesalius_ros` directory.

---

# Production environment
Production environment utilizes Docker Swarm to deploy and manage nodes across two host machines. Ensure that the machines are connected to the same router.

<!-- At least one of the host machines needs to be Linux, and in these instructions it will be used as the manager.-->  

The Docker Swarm configuration utilizes [weavenet](https://github.com/weaveworks/weave) to enable multicast across docker nodes. As stated in its documentation: "Weave Net creates a virtual network that connects Docker containers across multiple hosts and enables their automatic discovery."

### Terminology
- Manager: the host that runs the docker swarm and manages the services and workers
- Worker: a host that is controlled by the manager
- Service: a docker container that is running on some host. Our services all contain a single ros2 node

Note that node means different things in ros2 and docker terminology:  
Node in docker swarm = a host machine  
Node in ros2 = a ros2 node


### First time set up
1. If you have Windows hosts, install Docker Desktop and X Server (instruction in Docker -> Window section)
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
- Set docker-compose.prod.yml neuronavigation DISPLAY to `:0` or `:0.0`


After the previous steps are done, run the following commands to start the swarm cluster.
1. Host: Create docker swarm `docker swarm init`
2. Worker: Copy the docker swarm join command (`docker swarm join --token <token> <ip:port>`) produced by the previous command and run it on the worker  
3. Host: (Optional) Ensure that you can see both nodes in the network `docker node ls`
4. Host: Deploy docker stack `docker stack deploy -c docker-compose.prod.yml mtms --with-registry-auth`
5. Host: (Optional) Check service status `docker service ls` and `docker service ps --no-trunc <service>`
