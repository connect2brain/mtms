# MATLAB API for mTMS software

## Setting up

1. Install MATLAB
2. Install ROS Toolbox for MATLAB
3. Add `/home/mtms/repositories/mtms/api/matlab` to the path in MATLAB (change path according to your setup).
4. Run `ros2RegisterMessages('/home/mtms/repositories/mtms/api/matlab')` (change path according to your setup).
5. Check using `ros2 msg list` that the ROS messages are registered. There should be mTMS-specific message types such as ones starting with `mtms` string.

## Using API

### Getting started

Create an instance of MTMSApi class and start the mTMS device by running:

```
api = MTMSApi();
api.start_device();
```

See `example.m` for a more thorough example on how to use the API.

## Development

### Rebuilding ROS messages

When ROS interfaces in `ros2_ws/src/interfaces` directory change, the ROS message types
need to be rebuilt into MATLAB types.

It is recommended to do this in Ubuntu; in Windows, you may run into problems, such as ROS Toolbox not finding Cmake.

Here are the steps:

1. Install colcon: `sudo apt install python3-colcon-common-extensions -y`. Alternatively, just install CMake.
2. Install Python 3.9 (NB: Needs to be Python 3.9. ROS Toolbox requires that particular version.) If your system has another Python version installed, you install Python 3.9 using
Anaconda. Another option is to install Python 3.9 so that it coexists with another Python version.

NB: To be supported by ROS Toolbox, the installed version of Python 3.9 needs to be old enough.
For instance, 3.9.15 does not work. The easiest way to install an old version of Python 3.9.x
is to install it by building the source code.

3. Open MATLAB and run `!which python3.9`.
4. Run `pyenv('Version', '<the result from previous command>')`.
5. Run `ros2genmsg('/home/mtms/repositories/mtms/ros2_ws/src/interfaces', CreateShareableFile=true, BuildConfiguration='fasterruns')` (change path according to your setup).
6. Move `matlab_msg_gen.zip` to `api/matlab` directory in mTMS repository, replace the existing file.
7. Commit the updated ZIP file and push.

## Troubleshooting

### Problem: service calls do not finish

- MATLAB ROS Toolbox uses a different ROS Middleware Implementation (RMW) underneath than, for example, Galactic. Read the following discussion for more information:
https://se.mathworks.com/matlabcentral/answers/1777145-ros-toolbox-stuck-on-service-call.
To fix this issue, run the following command:
`setenv("RMW_IMPLEMENTATION","rmw_cyclonedds_cpp")`.

### ros2genmsg error: `No packages with '.msg' files found under [path to mTMS repository]/ros2_ws/src/interfaces/fpga_interfaces. Each message package directory must contain a directory named 'msg' that then contains '.msg' files.`
- Ensure that you are running ros2genmsg with the correct path; the path parameter should be, e.g., '/home/mtms/repositories/mtms/ros2_ws/src/interfaces' instead of '/home/mtms/repositories/mtms/ros2_ws/src/interfaces/fpga_interfaces'.
- Check that you do not have subdirectories inside any of the `msg` directories (however, there can be subdirectories inside `srv` directories).
- Check also that the paths to msg and srv files are correctly defined in CMakeLists.
