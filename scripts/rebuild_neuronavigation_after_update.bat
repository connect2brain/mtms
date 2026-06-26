@echo off
setlocal

REM === Paths for this computer ===
set "MTMS_ROOT=C:\Users\mtms\mtms"
set "CONDA_ROOT=C:\Users\mtms\anaconda3"
set "CONDA_ENV=ros_env"
set "VS_VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM === Load Visual Studio x64 compiler ===
call "%VS_VCVARS%"

REM === Activate Conda environment ===
call "%CONDA_ROOT%\Scripts\activate.bat" %CONDA_ENV%

echo.
echo === Checking Python and compiler ===
where python
python -c "import sys; print(sys.executable)"
where cl

echo.
echo === Go to MTMS repository ===
cd /d "%MTMS_ROOT%"

echo.
echo === Optional: update submodules ===
git submodule update --init --recursive

echo.
echo === Build ROS interfaces ===
python -m colcon build --base-paths interfaces

echo.
echo === Build external message/interface packages needed by neuronavigation ===
python -m colcon build --packages-select eeg_msgs targeting_msgs mep_interfaces

echo.
echo === Build invesalius_rs native Rust extension ===
cd /d "%MTMS_ROOT%\src\neuronavigation\invesalius3"

REM Avoid maturin using uv-created .venv instead of ros_env
if exist .venv (
    echo Renaming .venv to .venv_backup so maturin uses ros_env
    if exist .venv_backup rmdir /s /q .venv_backup
    rename .venv .venv_backup
)

cd /d "%MTMS_ROOT%\src\neuronavigation\invesalius3\invesalius_rs"

python -m pip install maturin
python -m pip uninstall -y invesalius-rs
python -m pip uninstall -y invesalius_rs

if exist target rmdir /s /q target

python -m maturin build --release

python -m pip install --force-reinstall "target\wheels\invesalius_rs-0.1.0-cp312-cp312-win_amd64.whl"

echo.
echo === Test Conda invesalius_rs ===
cd /d C:\Users\mtms
python -c "import invesalius_rs, invesalius_rs._native as n; print(invesalius_rs.__file__); print(n.__file__); print('polygon2mask_rs:', hasattr(n, 'polygon2mask_rs'))"

echo.
echo === Install InVesalius Python package without changing Conda dependencies ===
cd /d "%MTMS_ROOT%\src\neuronavigation\invesalius3"
python -m pip install --no-deps .

echo.
echo === Clean and rebuild neuronavigation ROS package ===
cd /d "%MTMS_ROOT%"

if exist build\neuronavigation rmdir /s /q build\neuronavigation
if exist install\neuronavigation rmdir /s /q install\neuronavigation

python -m colcon build --packages-select neuronavigation

echo.
echo === Source ROS workspace ===
call install\setup.bat

echo.
echo === Copy correct invesalius_rs native module into ROS-installed neuronavigation ===
copy /Y "%CONDA_ROOT%\envs\%CONDA_ENV%\Lib\site-packages\invesalius_rs\_native.cp312-win_amd64.pyd" "%MTMS_ROOT%\install\neuronavigation\Lib\site-packages\invesalius_rs\_native.cp312-win_amd64.pyd"

echo.
echo === Test ROS-installed invesalius_rs ===
python -c "import invesalius_rs, invesalius_rs._native as n; print(invesalius_rs.__file__); print(n.__file__); print('polygon2mask_rs:', hasattr(n, 'polygon2mask_rs'))"

echo.
echo === Done. Now you can run: ===
echo ros2 run neuronavigation start
echo.
pause
