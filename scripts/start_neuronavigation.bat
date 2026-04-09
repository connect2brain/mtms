@echo on
setlocal EnableDelayedExpansion

REM ==================================================
REM USER SETTINGS
REM ==================================================
set "VS_VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set "CONDA_ROOT=C:\Users\Admin\miniforge3"
set "ROBOT_CONTROL_ROOT=C:\Users\Admin\tms-robot-control"
set "MTMS_ROOT=C:\Users\Admin\mtms"
set "CONDA_ENV_PATH=%CONDA_ROOT%\envs\ros_env"
set "RELAY_PORT=5000"
set "DDS_INTERFACE_IP=192.168.200.223"

set "ENABLE_DASHBOARD=0"
set "DASHBOARD_ROOT=C:\Users\Admin\tms-experiment-dashboard"

REM ==================================================
REM 1) Load Visual Studio Toolchain
REM ==================================================
call "%VS_VCVARS_PATH%"
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to load Visual Studio environment
    pause
    exit /b 1
)

REM ==================================================
REM 2) Activate Conda Environment
REM ==================================================
call "%CONDA_ROOT%\Scripts\activate.bat" "%CONDA_ENV_PATH%"
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to activate Conda environment
    pause
    exit /b 1
)

REM ==================================================
REM 3) Force hardware OpenGL
REM ==================================================
set "QT_OPENGL=desktop"
set "LIBGL_ALWAYS_SOFTWARE=0"
set "MESA_LOADER_DRIVER_OVERRIDE="
set "GALLIUM_DRIVER="

set "CONDA_OGL=%CONDA_PREFIX%\Library\bin\opengl32.dll"
if exist "%CONDA_OGL%" (
    if not exist "%CONDA_PREFIX%\Library\bin\opengl32.dll.conda_bak" (
        echo Disabling conda OpenGL shim
        ren "%CONDA_OGL%" opengl32.dll.conda_bak
    )
)

REM ==================================================
REM 4) Start background services (SAFE start usage)
REM ==================================================
start "relay server" cmd /c python "%ROBOT_CONTROL_ROOT%\relay_server.py" %RELAY_PORT%

timeout /t 1 /nobreak >nul

start "main loop" cmd /c python "%ROBOT_CONTROL_ROOT%\main_loop.py"

REM ==================================================
REM 5) Prepare project
REM ==================================================
cd /d "%MTMS_ROOT%"

set "pkg_dir=%MTMS_ROOT%\src\neuronavigation\invesalius3"
set "init_file=%pkg_dir%\__init__.py"

if not exist "%pkg_dir%" mkdir "%pkg_dir%"
if not exist "%init_file%" type nul > "%init_file%"

REM ==================================================
REM 6) Build ROS package
REM ==================================================
colcon build --packages-select neuronavigation
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: colcon build failed
    pause
    exit /b 1
)

call install\local_setup.bat

REM ==================================================
REM 7) Optional dashboard
REM ==================================================
if "%ENABLE_DASHBOARD%"=="1" (
    start "TMS Dashboard" cmd /k ^
    "cd /d %DASHBOARD_ROOT% && call %CONDA_ROOT%\Scripts\activate.bat %CONDA_ENV_PATH% && python main.py"

    timeout /t 3 /nobreak >nul
    start "" http://localhost:8084
)

REM ==================================================
REM 8) Run ROS node
REM ==================================================
set "CYCLONEDDS_URI=<CycloneDDS><Domain><General><NetworkInterfaceAddress>%DDS_INTERFACE_IP%</NetworkInterfaceAddress></General></Domain>"

ros2 run neuronavigation start --ros-args ^
    -p electric_field_enable:=false ^
    -p robot_enable:=true

pause
