# Usage

Designed to work with Linux. Works also on WSL, but memory and scheduling optimizations must be disabled.

## ROS2 arguments
- `log-level`: DEBUG, INFO, WARN, or ERROR. Can also be lower case. 
- `processor-type`: python, matlab, or compiledmatlab. Defines the type of the processor to use
- `processor-script`: path to the processor script. See examples below
- `loop-count`: how many times to run the example measure function. If 0, it won't be run at all
- `file`: filename for saving durations. WARNING: existing files will be overwritten

## Python
`ros2 launch eeg_processor eeg_processor.launch.py log-level:=info processor-type:=python processor-script:=hotswappable_processors.python.python_processor loop-count:=1 file:=example.data`

## Compiled MATLAB
`ros2 launch eeg_processor eeg_processor.launch.py log-level:=info processor-type:=compiledmatlab processor-script:=../../hotswappable_processors/cppmatlab/compiler/libprocessor_factory.so loop-count:=100000 file:=example.data`

## MATLAB (slow, not recommended)
`ros2 launch eeg_processor eeg_processor.launch.py log-level:=info processor-type:=matlab processor-script:==../../hotswappable_processors/matlab/ loop-count:=0 file:=example.data`


#### Installation

The variable :variable:`Matlab_ROOT_DIR` may be specified in order to give
the path of the desired Matlab version. Otherwise, the behaviour is platform
specific:

* Windows: The installed versions of Matlab are retrieved from the Windows registry
* OS X: The installed versions of Matlab are given by the MATLAB paths in ``/Application``. If no such application is
  found, it falls back to the one that might be accessible from the PATH.
* Unix: The desired Matlab should be accessible from the PATH. This means the matlab/bin folder

