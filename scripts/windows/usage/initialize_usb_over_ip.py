import time
import subprocess
import re
import numpy as np

print("Initializing USB over IP.")

cmd = "usbipd wsl list"
process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
output, error = process.communicate()

output = output.decode("utf-8") 
lines = output.split('\n')

def attach_device(device_name):
    i = np.where([True if re.search(device_name, l) else False for l in lines])[0][0]

    polaris_line = lines[i]

    busid = polaris_line.split(" ")[0]

    cmd = "usbipd wsl attach --busid {}".format(busid)
    process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()

attach_device("NDI Host USB")  # Polaris
attach_device("USB Serial Device")  # USB pedal

print("Done.")
