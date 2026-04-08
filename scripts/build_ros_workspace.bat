@echo off

REM This script is used to build the neuronavigation-relevant parts of the ROS2 workspace on the Windows computer
REM in the lab in Aalto and Tubingen, including the neuronavigation package itself.
REM
REM To just build neuronavigation without its dependencies (i.e., the ROS interfaces), use build_neuronavigation.bat.

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

REM call C:\dev\ros2_jazzy\local_setup.bat

REM ==================================================
REM 2) Activate Conda Environment (ros_env)
REM ==================================================
call "C:\Users\Admin\miniforge3\Scripts\activate.bat" "C:\Users\Admin\miniforge3\envs\ros_env"
 
IF %ERRORLEVEL% NEQ 0 (
    echo ❌ Failed to activate Conda environment
    pause
    exit /b 1
)

cd %MTMS_ROOT%

REM Ensure __init__.py exists for Python package discovery in invesalius3
set "pkg_dir=%MTMS_ROOT%\src\neuronavigation\invesalius3"
set "init_file=%pkg_dir%\__init__.py"

if not exist "%pkg_dir%" (
  echo Creating directory: "%pkg_dir%"
  mkdir "%pkg_dir%"
)

if not exist "%init_file%" (
  echo Creating __init__.py: "%init_file%"
  type nul > "%init_file%"
)

colcon build --base-paths interfaces

REM Ask for a keypress before closing the terminal window so that potential errors are shown to the user.
pause
