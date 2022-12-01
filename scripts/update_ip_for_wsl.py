import time
import subprocess
import re
import numpy as np

# HACK: Extra waiting period is needed here if this script is run in startup so that
#       WSL has time to start up first.

print("Waiting for WSL to start up...")

time.sleep(10)

# Determine IP for WSL

print("Updating IP for WSL.")

cmd = "ipconfig"
process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
output, error = process.communicate()

output = output.decode("utf-8") 

splitted = output.split('\n')
ipv4_lines = [l for l in splitted if re.search("IPv4", l)]

wsl_ip_line = ipv4_lines[1]
wsl_ip = wsl_ip_line.split(": ")[1].strip()

# Update .env file

env_file_path = r'C:\Users\mtms\mtms\.env'

f = open(env_file_path, 'r')
lines = f.readlines()
f.close()

i = np.where([True if re.search("DISPLAY", l) else False for l in lines])[0][0]
lines[i] = "DISPLAY={}:0.0\n".format(wsl_ip)

f = open(env_file_path, 'w')
f.writelines(lines)
f.close()

print("Done.")
