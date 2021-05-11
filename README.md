# Project Louhi

This project contains the git repository for development of the mTMS system in C2B.

## Windows

#### Installing InVesalius

- Clone the repository by running:

```
git clone https://github.com/okahilak/invesalius3 --branch add-backend-communication-to-tms-mode
```

- Note that you need 64-bit version of Python 3.7 to be able to easily install all the required
Python libraries. One way of achieving this is to install Python 3.7 from

```
https://www.python.org/downloads/release/python-379/
```

to a separate directory, such as `C:\Python37`. When installing, deselect "Install launchers for
all users" checkbox.

When using this Python version, prefix Python and pip commands with the explicit directory,
such as `C:\Python37\Scripts\pip install ...` to ensure that the correct version is used.

- Set up the software by following the instructions on the following page:

```
https://github.com/invesalius/invesalius3/wiki/Running-InVesalius-3-in-Windows
```

NB: If Visual Studio Community Edition is installed for setting up InVesalius properly,
one set of installation options that seems to work is: Python, Node.js, .NET, C++, and
Universal Windows.

- Install additional libraries needed by InVesalius-mTMS communication by running:

```
C:\Python37\Scripts\pip install python-socketio requests
```

#### Installing GNU tools

- Install Make for Windows from http://gnuwin32.sourceforge.net/packages/make.htm.
Download and run "Complete package, except sources".

- Add the installation directory for the tool into path environment variable (the default installation
directory: `C:\Program Files (x86)\GnuWin32\bin`).

#### Installing cmder

- Install cmder from https://cmder.net/ to enable Unix-style command line. Mini version
suffices, but Full version can be installed if desired.

#### Installing LabVIEW

- LabVIEW can be installed from National Instrument's website.

- Version: 2019 service pack 1

NB: If you install LabVIEW to some other than the default directory, you may have to copy
nipythonhost.exe from `D:\custom_directory\National Instruments\Shared\NIPythonInterface` to
`D:\custom_directory\National Instruments\LabVIEW 2020` for LabVIEW to be able to run Python
processes.

#### Setting up the system

- Open a terminal window by running `cmder`.

- Run the following commands in the terminal window in project-louhi directory:

```
pip install python-dotenv
```

NB: Cmder needs to be run as the administrator for these commands to work.

NB: Python-dotenv is needed for running the mTMS bridge in LabVIEW.

- Initialize the environment variables by doing the following changes to `.env` file in project-louhi
directory:

    - Change the line `LOG_DIRECTORY=/tmp/logs` to `LOG_DIRECTORY=C:\logs` or similar.

### Getting started

- Open a terminal window by running `cmder`.

- Start all services by running:

    `docker-compose up -d`

- Then, you can check that all services are successfully started by checking
that there are no errors in the logs:

    - `docker-compose logs --tail=100 -f [container]`

where `[container]` is one of the following: `zookeeper`, `kafka`, `backend`,
`frontend`, `mtms_bridge`.

- Start InVesalius by running the following in InVesalius repository:

```
C:\Python37\python app.py
```

## Debian and Ubuntu

### Installation

Tested using Ubuntu 20.04.

#### Installing InVesalius

- Clone the repository by running:

```
git clone https://github.com/okahilak/invesalius3 --branch add-backend-communication-to-tms-mode
```

- Set up the software by following the instructions on the following page:

```
https://github.com/invesalius/invesalius3/wiki/Running-InVesalius-3-in-Linux
```

- Install additional libraries needed by InVesalius-mTMS communication by running:

```
pip3 install python-socketio
```

### Getting started

- Start all services by running:

    `sudo docker-compose up -d`

- Then, you can check that all services are successfully started by checking
that there are no errors in the logs:

    - `sudo docker-compose logs --tail=100 -f [container]`

where `[container]` is one of the following: `zookeeper`, `kafka`, `backend`,
`frontend`, `mtms_bridge`.

- Start InVesalius by running the following in InVesalius repository:

```
python3 app.py
```

## Examples

### Example: Updating stimulation parameters in the frontend

- Open the frontend at http://localhost:8081/. Navigate to TMS page and
change the intensity. (Please refresh the page first after navigating to
TMS page, as there is a bug that the connection is not shown until the
page is refreshed.)

- Check from the backend logs that the parameter value has been updated:

    - `sudo docker-compose logs --tail=100 -f backend`

### Example: Changing and listening to the stimulation parameters directly via backend

- Open a terminal window and run the following command in project-louhi directory:

```
python3 examples/parameter_client.py
```

NB: For this to work, you need to have python-socketio package installed in Python.

- When the script notifies that it has connected to the backend, write to the command line
one or several of the following messages:

```
intensity 200
iti 100
number_of_stimuli 10
```

- Check that the script echoes the new parameter value back, indicating that the parameters
have been updated in Kafka.

### Example: Changing parameter values in InVesalius

- Start InVesalius by running the following in InVesalius repository:

```
python3 app.py
```

or (if you are running Windows):

```
C:\Python37\python app.py
```

- Enable TMS mode in InVesalius (Mode -> Navigation Mode -> Transcranial Magnetic Stimulation Mode).

- Open an existing project, such as the example project `Cranium.inv3`.

- Select `5. Navigation System` from the menu on the left. Open `Transcranial Magnetic Stimulation` submenu.

- Press "Connect". The parameter values, such as intensity, should now be updated in their corresponding
fields.

- Change parameter values and check that the values are echoed back to InVesalius.

### Example: InVesalius-mTMS communication

- Start InVesalius, as in the previous example.

- Start LabVIEW and open mTMS software. Open SubVI called `api` under API folder.
Set Module path to the file `bridge\labview_runner.py` in project-louhi directory.
Run the SubVI.

- Send commands, such as Stimulate and Recharge, and check that the commands end up in the mTMS software.

## Other

### Makefile

A Makefile is provided, including make targets to, e.g., reset environment variables to their
default values.

Run `make help` for a list of make targets.

### Sandbox

Sandbox is used to store experimental scripts, drafts, etc.
