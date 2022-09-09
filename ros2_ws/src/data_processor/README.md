# Usage

`ros2 launch data_processor data_processor.launch.py processor-type:=python processor-script:=processors.python_processor log-level:=info loop-count:=10000`

` ros2 launch data_processor data_processor.launch.py log-level:=info processor-type:=cpp processor-script:=/home/alqio/workspace/mtms/ros2_ws/src/processors/matlab/compiler/libprocessor_factory.so loop-count:=100000`

Add MATLAB install folder to path

# MATLAB

The variable :variable:`Matlab_ROOT_DIR` may be specified in order to give
the path of the desired Matlab version. Otherwise, the behaviour is platform
specific:

* Windows: The installed versions of Matlab are retrieved from the Windows registry
* OS X: The installed versions of Matlab are given by the MATLAB paths in ``/Application``. If no such application is
  found, it falls back to the one that might be accessible from the PATH.
* Unix: The desired Matlab should be accessible from the PATH. This means the matlab/bin folder