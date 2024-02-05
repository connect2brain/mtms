# Installing mTMS real-time computer

Assuming that the real-time computer has a pre-installed Windows 11.

## Configure router (Edgerouter X)

- Power on router
- Connect router to the real-time computer (use eth0)
- In Windows, disable DHCP and manually set up computer to use IP 192.168.1.2 and network mask 255.255.0.0.
- Connect to 192.168.1.1 using web browser
- The default username/password is ubnt/ubnt.
- When asked, select to use setup wizard. Set internet port to eth0. Reset username and password to ubnt/ubnt (the same default username and password, but the UI requires rewriting them). Don’t do other changes.
- Reboot router from the UI.
- While rebooting, switch the ethernet cable from the real-time computer to connect to eth1, and connect the ethernet cable from the internet to eth0.
- Connect ethernet cable from Bittium NeurOne to eth2 (this is important so have the same configuration on all installations).
- Enable DHCP on real-time computer.

## Disable BitLocker encryption on Windows 11

- Write `Data encryption` to the text field next to the magnifying glass. Disable encryption.

## Update and configure BIOS

- Write `HP Support Assistant` to the text field next to the magnifying glass. Open the support assistant. Go to `Device Support` and select `Software & Drivers`. Select `HP Consumer Desktop PC BIOS Update`. Follow the instructions.
- Go to BIOS settings by pressing Esc during booting (on HP Envy, other computers might use a different hotkey). Change BIOS language to English. Disable secure boot.

## Install dual-booting Windows and Ubuntu

- Boot to Windows. Open `Disk Management`. Shrink the main partition (assumedly taking up the whole disk space) to 200 GB, leaving the rest unallocated. (The plain Windows 11 installation may take up to 85 GB or so, and LabVIEW may take tens of GBs - that’s why we’re leaving as much as 200 GB for Windows.)
- Boot from a memory stick with Ubuntu 22.04.3 LTS by pressing Esc while booting and following the instructions.
- Select `Try or install Ubuntu`. If you encounter trouble with Nouveau drivers, crashing during start-up, do the following steps:
Instead of pressing enter while `Try or install Ubuntu` is selected, press ‘e’.
Change the line:
linux /casper/vmlinuz ... quiet splash ---
	to:
		linux /casper/vmlinuz … quiet splash nomodeset —
Proceed by pressing f10 or ctrl-x.

- Follow the instructions. When asked, select `Minimal installation` to minimize the potential for conflicting drivers or system configuration etc. Ensure that the checkboxes for `Download updates while installing Ubuntu` and `Install third-party software for graphics and Wi-Fi hardware and additional media formats` are ticked.
- When asked about Installation Type, select `Something else`. This will allow you to set the partitions for the Ubuntu installation by yourself.
- There should be a device called `Free space` with a large size (such as 800 G). Select that and press `+`.
- Root partition: Set the partition size to 120000 MB. Set type to `Primary` and location to `Beginning of this space`. Set file system to `Ext4 journaling file system`. Set mount point to `/` (without the quotes). Select `Ok`.
- Home partition: Set the partition size to the maximum available size. Set type to `Primary` and location to `Beginning of this space`. Set file system to `Ext4 journaling file system`. Set mount point to `/home` (without the quotes). Select `Ok`.
- Select /dev/nvme0n1p1 (Windows Boot Manager) as `Device for boot loader installation` to ensure that dual-booting works. Proceed.

- When asked, set `mtms` as your name. The username is automatically set to match that.
- Set `multilocus` as the password.
- Select the option `Require my password to log in`.
- Follow the instructions to finish the installation.

## Install real-time Ubuntu patch

- When logging into Ubuntu for the first time, you are asked about updating to Ubuntu Pro. Agree to update and follow the instructions. Create a Ubuntu Pro account (personal one) and finish the update.
- After Ubuntu Pro is installed, write the following on the command line: sudo pro enable realtime-kernel. Follow the instructions. Reboot.

## Install NI drivers

- Go to: https://www.ni.com/docs/en-US/bundle/ni-platform-on-linux-desktop/page/installing-ni-drivers-and-software-on-linux-desktop.html
- Go under `Related information` -> `Linux Device Driver Download`. Select the latest (e.g., 2024 Q1) and press download. Note: In Aalto and Chieti, 2024 Q1 and 2023 Q4 drivers are currently installed, respectively.

- Unzip the ZIP file.
- Go to: https://www.ni.com/docs/en-US/bundle/ni-platform-on-linux-desktop/page/installing-ni-products-ubuntu.html
- Follow the instructions on the page. Replace `filename.deb` in the instructions with `ni-ubuntu2204-drivers-2024Q1.deb`, which is one of the files inside the .zip file you unzipped in the previous step. Ensure that you install the correct drivers: they should be for Ubuntu 22.04 (determined by `2204` in the filename).
- Step 5 does not specify which packages you need to install. Here is the list of the packages needed for our use:

sudo apt install ni-rseries

- After that, continue from step 6.

## Install Git

- Run `sudo apt install git` on the command line.

## Install Git LFS

- Go to: https://git-lfs.com
- Follow the instructions. Here are the essential steps:

curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
sudo apt install git-lfs
git lfs install

- If Git LFS is not installed, the bitfile won’t load into the FPGA and you’ll encounter trouble in the later steps.

## Clone mTMS repository

- Create a ssh-key by running `ssh-keygen -o`. You can use your personal passphrase and personal GitHub account during the installation. Later on, the site should manage the repository using their own GitHub account.
- Log into GitHub. Go to Settings -> SSH and GPG keys -> New SSH key. Copy the contents of .ssh/id_rsa.pub to `Key` field. Give a name for the key, e.g., `Chieti mTMS computer`.
- Clone the mTMS repository by running in the home directory:

git clone git@github.com:connect2brain/mtms.git --recurse-modules

## Create environment file for the site (only when setting up a new installation)

- Go to mtms repository root.
- Create the directory for the site, e.g.,

mkdir sites/Chieti

- Copy an existing .env file for the new site, e.g.,

cp sites/Tubingen/.env sites/Chieti/

- Run `git add sites/[site-name]`
- Run `git commit`
- Ensure that the changes are pushed and merged.

## Run mTMS setup scripts

- Go to directory scripts/linux/installation in the mTMS repository.
- Run:

sudo ./setup-lab-computer [site-name]

For example:

sudo ./setup-lab-computer Chieti

If setting up a computer for development, run:

sudo ./setup-lab-computer dev

- Manually modify /etc/systemd/system/mtms-core-containers.service and /etc/systemd/system/mtms-device-bridge.service to have the correct site, e.g.:

sudo nano /etc/systemd/system/mtms-core-containers.service
[modify site on line `Environment=SITE=Tubingen`, e.g., to `Environment=SITE=Chieti`]
[save and exit]

Similarly for /etc/systemd/system/mtms-device-bridge.service

Run `sudo systemctl daemon-reload`

- Set up the .bashrc by running in the same directory (scripts/linux/installation) - note the lack of sudo:

./setup-environment [site-name]

## Build containers

- Reboot to automatically build Docker containers. Building the containers takes some time; its progress can be followed by running `log` on the command line.

- After finished, ensure that the containers started without trouble by checking that `log` command does not show major errors.

- Also ensure that the mTMS device bridge started without errors by running `sudo journalctl -u mtms-device-bridge -f` on the command line.

## Create example project

- On the command line, run `create example` to create the example project.

Create desktop link to Web UI

- Open Google Chrome, go to: https://localhost:3000
- Press the download-link style button in the top right corner (`Install mTMS panel`). Press `Install`.
- Go to desktop, right click on the file with the title `chrome-[something]`. Select `Allow Launching`.

## Set up Bittium NeurOne (on site computer)

- Go to `Network` settings in Ubuntu, open the wired settings -> IPv4. Set IPv4 Method to `Automatic (DHCP)`. Set DNS and Routes to Automatic. Apply and reboot if the settings were changed.

- Check the network interface name on the real-time computer by running ifconfig. It should be something like `enp60s0`.
- Run `nmcli con show` to see what is the name of the network profile that the network interface is using, e.g., `Wired connection 1`.
- Run:

nmcli con mod `Wired connection 1` +ipv4.addresses `192.168.200.221/24`

where `Wired connection 1` is replaced by the network profile name as shown by the previous nmcli con show command.

- Run: sudo systemctl restart NetworkManager
- Run:

ip addr show

to check that both IPs (the one provided by DHCP and 192.168.200.221) are present under the interface name.

- Start streaming in NeurOne. Run `sudo wireshark` and open the correct network interface. Check that packets are received from the IP 192.168.200.220.

## Set up Bittium NeurOne (development computer)

- Go to `Network` settings in Ubuntu, open the wired settings -> IPv4. Set IPv4 Method to `Manual`. Set addresses fields to:

Address: 192.168.200.221
Netmask: 255.255.255.0
Gateway: -

- Disable Automatic DNS and Automatic Routes
- Select `Apply`
- Reboot

## Configure Grub timeout

- On the command line, run `sudo nano /etc/default/grub`
- Modify `GRUB_TIMEOUT=0` line to `GRUB_TIMEOUT=1`
- Run `sudo update-grub`
- Reboot

## Install NVIDIA display driver (for two-display setup)

- The automatically installed NVIDIA display driver don’t work with PREEMPT_RT kernel patch, causing the GPU (NVIDIA RTX3080 at Aalto) to not recognize two displays. Here are the instructions to manually install the display driver.

- Download a .run file for manual installation: https://nvidia.com/Download/index.aspx

- Purge the automatically installed drivers: sudo apt-get purge nvidia*

- Disable Nouveau (open-source) drivers: 1) sudo nano /etc/modprobe.d/blacklist.conf, 2) Add line `blacklist nouveau` to the file and save.

- Disable Nouveau kernel module:

sudo bash -c "echo options nouveau modeset=0 > /etc/modprobe.d/nouveau-kms.conf"
sudo update-initramfs -u

- Press ctrl+alt+F6 to enter a text-based terminal.

- Log in, stop display manager: sudo systemctl stop gdm

- Go to the directory where the NVIDIA .run file was downloaded to. Run:

chmod +x NVIDIA-Linux-*.run
sudo IGNORE_PREEMPT_RT_PRESENCE=1 ./NVIDIA-Linux-[driver-number].run

Select `yes` to all of the questions.

- Run `sudo reboot`

- After restarting, the second display should be automatically recognized and taken into use.

## Install MATLAB

- Install MATLAB with the following toolboxes: ROS toolbox, Statistics and machine learning toolbox.

- Open MATLAB by running:

```
sudo matlab
```

This ensures that you can change all configuration parameters, such as the path.

- Open HOME -> Set Path -> Add Folder. Add the folder `/home/mtms/mtms/api/matlab`.
Open Keyboard -> Shortcuts and change the value of "Active settings" to "Windows Default Set".
Apply changes.

## Build documentation locally

The documentation source files are located in `docs` and are built using [Sphinx](https://www.sphinx-doc.org/en/master/usage/quickstart.html).

To build the documentation locally, first make sure your environment has completed the installation steps described [above](#installation). Particularly, make sure you have ROS activated: e.g. in Ubuntu 22.04/bash, use `source /opt/ros/iron/setup.bash`.

Subsequently, install the Sphinx dependencies
```
python3 -m pip install -r docs/sphinx-requirements.txt
```

Finally, enter the `docs` folder and build the documentation
```
cd docs
make clean
make html
```

The documentation can then be found in `docs/build/html/` folder and can be viewed with your favorite web browser. A natural place to start is `docs/build/html/index.html`.

## Install InVesalius (only in Chieti)

- Install the dependencies needed by InVesalius3 (see installation instructions in [Wiki](https://github.com/invesalius/invesalius3/wiki))

- Build the Cython modules needed by InVesalius3 (likewise, see installation instructions). Note that InVesalius
  resides in the directory `invesalius_ros/ros2_ws/src/neuronavigation/neuronavigation/invesalius3`, therefore the modules
  need to be built in that directory.

- You can check that the installation succeeded by running `python3 app.py` in the above directory and checking
  that InVesalius starts.

## Install Visual Studio Code (optional)

- Go to: https://code.visualstudio.com/docs/setup/linux
- Follow the instructions.

## Install Google Chrome (optional)

- Go to: https://www.google.com/chrome/?platform=linux
- Download the .deb package.
- Run `sudo apt install ./google-chrome-stable_current_amd64.deb`

## Install Git Credential Manager (optional)

- Install Git Credential Manager (GCM):

```
wget "https://github.com/GitCredentialManager/git-credential-manager/releases/download/v2.0.935/gcm-linux_amd64.2.0.935.deb" -O /tmp/gcmcore.deb
sudo dpkg -i /tmp/gcmcore.deb
git-credential-manager configure
git config --global credential.credentialStore secretservice
```

When asked, select the option: "Sign in with your browser" and enter your GitHub username and password.

# Troubleshooting

## Fix network interface switching names (only happens in Chieti)

- The problem: the primary network interface kept switching names between enp60s0 and enp70s0, depending on if there is another computer connected to the same router or not. Add the following line to /etc/udev/rules.d/10-network.rules file:

SUBSYSTEM=="net", ACTION=="add", ATTR{address}=="6c:02:e0:52:4e:43", NAME="enp60s0"

- Reboot the system.
