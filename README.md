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

- Possibly also follow [these](https://github.com/microsoft/WSL/issues/6430) instructions but replace VcXsrv with Xming. So Control Panel > System and Security > Windows Defender Firewall > Advanced Settings > Inbound Rules > New Rule... > Program > %ProgramFiles%\Xming\Xming.exe > Allow the connection > checked Domain/Private/Public > Named and Confirmed Rule.

- Run the rest of the commands in WSL

- Install Docker and Docker compose

- Check your `$DISPLAY` variable for example with `echo $DISPLAY`. Update .env file DISPLAY to that. Most likely it's enough to set it just as `:0`.

- Run the following commands on the root directory of the repository

```
xhost local:root
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
