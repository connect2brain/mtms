# Introduction

EEG processor is collection of three nodes:

| Node               | Input          | Output      |
|--------------------|----------------|-------------|
| EEG preprocessor   | Raw EEG        | Cleaned EEG |
| EEG processor      | Cleaned EEG    | Events      |
| Stimulus presenter | Stimuli events | Events      |

All nodes share the same architecture which allows them to dynamically load algorithms. This design allows users of the
mTMS system to use arbitrary algorithms for closed-loop control of the mTMS device.

# Usage

Designed to work with Linux. Works also on WSL, but memory and scheduling optimizations must be disabled manually.

EEG processor collection has 4 launch configurations.

1. all_stages.launch.py, which launches all 3 nodes.
2. eeg_preprocessor.launch.py, which launches only EEG preprocessor.
3. eeg_processor.launch.py, which launches only EEG processor.
4. stimulus_presenter.launch.py, which launches only Stimulus presenter.

Each node must be given the type and the path of the hot-swappable algorithm as ROS2 launch arguments. If launching all
3 nodes simultaneously with all_stages.launch.py, all of the following arguments are required (with the exception
of `preprocess`, as it's implied if starting all).

All:

- `log-level`: DEBUG, INFO, WARN, or ERROR. Can also be lower case.

EEG preprocessor:

- `preprocessor-type`: python, matlab, or compiledmatlab. Defines the type of the preprocessor to use
- `preprocessor-script`: path to the preprocessor script. See examples below

EEG processor:

- `processor-type`: python, matlab, or compiledmatlab. Defines the type of the processor to use
- `processor-script`: path to the processor script. See examples below
- `preprocess`: Specifies whether EEG processor should subscribe to raw or cleaned EEG data. If true, subscribes to
  cleaned data.

Stimulus presenter:

- `stimulus-presenter-type`: python, matlab, or compiledmatlab. Defines the type of the hot swappable component to use
  for stimulus present
- `stimulus-presenter-script`: path to the hot swappable script. See examples below

## Hot-swappable algorithms

Dynamically loaded hot-swappable algorithms can be written in Python, MATLAB, or CPP. The MATLAB versions must be
compiled
into C++ with MATLAB Coder. Here are examples how to specify each type of hot-swappable algorithm. Note that EEG
processor is used as an example here, but the same applies for EEG preprocessor and Stimulus presenter. For example,
for EEG preprocessor, simply replace `processor-type` with `preprocessor-type`.

### Python

- `processor-type:=python`
- `processor-script:=hotswappable_processors.python.python_processor`

### Compiled MATLAB

- `processor-type:=compiledmatlab`
- `processor-script:=/home/mtms/workspace/mtms/hotswappable_processors/cppmatlab/compiler/libprocessor_factory.so`

### CPP

- `processor-type:=cpp`
- `processor-script:=/home/mtms/workspace/mtms/hotswappable_processors/cpp/libprocessor_factory.so`

### MATLAB (slow, not recommended, only for legacy reasons)

- `processor-type:=matlab`
- `processor-script:=/home/mtms/workspace/mtms/hotswappable_processors/matlab/`

## Installation

The variable :variable:`Matlab_ROOT_DIR` may be specified in order to give
the path of the desired Matlab version. Otherwise, the behaviour is platform
specific:

* Windows: The installed versions of Matlab are retrieved from the Windows registry
* OS X: The installed versions of Matlab are given by the MATLAB paths in ``/Application``. If no such application is
  found, it falls back to the one that might be accessible from the PATH.
* Unix: The desired Matlab should be accessible from the PATH. This means the matlab/bin folder

