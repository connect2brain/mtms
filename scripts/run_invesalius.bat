@echo on
setlocal EnableDelayedExpansion

REM ==================================================
REM 1) Load Visual Studio 2019 x64 Toolchain
REM ==================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
 
IF %ERRORLEVEL% NEQ 0 (
    echo ❌ Failed to load Visual Studio environment
    pause
    exit /b 1
)

REM ==================================================
REM 2) Activate Conda Environment (ros_env)
REM ==================================================
call "C:\Users\Admin\miniforge3\Scripts\activate.bat" "C:\Users\Admin\miniforge3\envs\ros_env"
 
IF %ERRORLEVEL% NEQ 0 (
    echo ❌ Failed to activate Conda environment
    pause
    exit /b 1
)
 
start "relay server" call python C:/Users/Admin/tms-robot-control/relay_server.py 5000
 
timeout /t 1 /nobreak
 
cd /d C:\Users\Admin\tms-robot-control
 
start "main loop" call python C:/Users/Admin/tms-robot-control/main_loop.py
 

 

 
REM ==================================================
REM 3) REMOVE BAD LINKERS (Critical for Rust/MSVC)
REM ==================================================
set "PATH=%PATH:C:\Users\Admin\miniforge3\envs\ros_env\Library\usr\bin;=%"
set "PATH=%PATH:C:\Program Files\Git\usr\bin;=%"
 
REM ==================================================
REM 4) Verify Correct Linker
REM ==================================================
echo.
echo ==== Checking linker ====
where link
echo ==========================
echo.
 
for /f "tokens=*" %%i in ('where link') do (
    echo %%i | find "Microsoft Visual Studio" >nul
    if !errorlevel! == 0 set VS_LINK_FOUND=1
)
 
if not defined VS_LINK_FOUND (
    echo ❌ ERROR: Visual Studio linker not first in PATH
    pause
    exit /b 1
)
 
REM ==================================================
REM 5) Set Project Root
REM ==================================================
set MTMS_ROOT=C:\Users\Admin\mtms
cd /d %MTMS_ROOT%
 
REM ==================================================
REM 6) Ensure __init__.py exists
REM ==================================================
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
 
 
REM ==================================================
REM 8) Build ROS Package
REM ==================================================
colcon build --packages-select neuronavigation
 
IF %ERRORLEVEL% NEQ 0 (
    echo ❌ colcon build failed
    pause
    exit /b 1
)
 
call install\local_setup.bat
 
REM start "TMS Dashboard" cmd /k "cd /d C:\Users\Admin\tms-experiment-dashboard && call C:\Users\Admin\miniforge3\Scripts\activate.bat C:\Users\Admin\miniforge3\envs\ros_env && python main.py"
REM timeout /t 3 /nobreak >nul
REM start "" http://localhost:8084
 
REM ==================================================
REM Run Node
REM ==================================================
set "CYCLONEDDS_URI=<CycloneDDS><Domain><General><NetworkInterfaceAddress>192.168.200.223</NetworkInterfaceAddress></General></Domain>"
ros2 run neuronavigation start --ros-args -p electric_field_enable:=false -p robot_enable:=true
 
pause