REM This script is used to start neuronavigation on the Windows computer in the lab in Aalto. Its contents are
REM bound to change; for instance, we currently have to run neuronavigation outside Docker to be able to use
REM Optitrack motion tracking. Once Optitrack works with Docker, this can be changed to just run something like:
REM
REM docker-compose up neuronavigation

@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

call C:\dev\ros2_iron\local_setup.bat

cd %MTMS_ROOT%\ros2_ws

REM Note that neuronavigation_interfaces and ui_interfaces are NOT rebuilt to save some time when starting neuronavigation.
REM
REM Before running this for the first time, ensure that you have run manually:
REM
REM colcon build --packages-select neuronavigation_interfaces ui_interfaces
colcon build --packages-select neuronavigation
call install\local_setup.bat

ros2 run neuronavigation start
