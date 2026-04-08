## ROS2 environment
* Install Visual Studio Build Tools 2019 and 2022 for C++ development(**both!**)(the default install for C++ should be sufficient)



* [Follow latest robostack instructions for installation](https://robostack.github.io/GettingStarted.html#__tabbed_1_2):

  * Install Anaconda (Robostack recommends Miniforge installation)
  * **Note**: After reinstalling with Miniforge the ROS environment worked properly on Windows 11 
  * Do not install Python separately and make sure there are no entries to PATH (could be a source of bugs during installation/building) 
  * Remove default anaconda channels 

    * might need to edit .condarc files to get rid of them
  - Run the commands from the instructions

 

## Clone repositories
* Clone latest tms-robot-control to `C:\Users\<username>` (https://github.com/biomaglab/tms-robot-control)
* Clone latest mtms software to `C:\Users\<username>` (https://github.com/connect2brain/mtms)
  * `git clone --recurse-submodules` to automatically clone latest invesalius3 as well
* Clone invesalius3 if not already cloned  to `C:\Users\<username>\mtms\src\neuronavigation`(replace the empty folder)


## Build ROS2 workspace
* Note!!!!: Turn windows smart app control off, will prevent the build
* Set Windows system/user environment varialbe `MTMS_ROOT` to `C:\Users\<username>\mtms`
* Use `mtms\scripts\build_ros_workspace.bat` script if possible (run it in the conda environment)
  * Check that the paths are set correctly in the script, including the Visual Studio binary path (BuildTools vs Community)
* If doesn't work try manually 
  * create empty `__init__.py` file in invesaulius3
  * `conda activate ros_env`
  * `cd %MTMS_ROOT%` 
  * `colcon build --base-paths interfaces`

## Build invesalius3

Before building need to set environment variables 

Copy and paste `c:\Users\<username>\Tms-robot-control\.env.example` and set the variables in the file
Change filename to `.env`

In the ros_env navigate to invesalius folder and run the commands:

```
pip install uv
```
```
pip install maturin
```
```
uv sync
```
```
maturin develop --release
```
If develop doesn't work (currently a known bug):
```
maturin build --release
```
Install dependencies to the ros_env (currently needed to change `pyproject.toml` -> "vtk>=9.3.0", "setuptools>=68")
```
pip install .
```

To run invesalius, use the new .bat file in scripts folder `run_invesalius.bat`


