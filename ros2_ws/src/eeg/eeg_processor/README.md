# Usage

Designed to work with Linux. Works also on WSL, but memory and scheduling optimizations must be disabled.

## ROS2 arguments
- `log-level`: DEBUG, INFO, WARN, or ERROR. Can also be lower case. 
- `preprocessor-type`: python, matlab, or compiledmatlab. Defines the type of the preprocessor to use
- `preprocessor-script`: path to the preprocessor script. See examples below
- `processor-type`: python, matlab, or compiledmatlab. Defines the type of the processor to use
- `processor-script`: path to the processor script. See examples below
- `stimulus-presenter-type`: python, matlab, or compiledmatlab. Defines the type of the hot swappable component to use for stimulus present
- `stimulus-presenter-script`: path to the hot swappable script. See examples below

## Python
`processor-type:=python processor-script:=hotswappable_processors.python.python_processor`

## Compiled MATLAB
`processor-type:=compiledmatlab processor-script:=/home/mtms/workspace/mtms/hotswappable_processors/cppmatlab/compiler/libprocessor_factory.so`

## MATLAB (slow, not recommended)
`processor-type:=matlab processor-script:==/home/mtms/workspace/mtms/hotswappable_processors/matlab/`


## Installation

The variable :variable:`Matlab_ROOT_DIR` may be specified in order to give
the path of the desired Matlab version. Otherwise, the behaviour is platform
specific:

* Windows: The installed versions of Matlab are retrieved from the Windows registry
* OS X: The installed versions of Matlab are given by the MATLAB paths in ``/Application``. If no such application is
  found, it falls back to the one that might be accessible from the PATH.
* Unix: The desired Matlab should be accessible from the PATH. This means the matlab/bin folder

