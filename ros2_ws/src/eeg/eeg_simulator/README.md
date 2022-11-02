## EEG simulator

### Running

1. In directory `ros2_ws`, run `colcon build --packages-select eeg_simulator`.

2. Run `ros2 launch eeg_simulator eeg_simulator.launch.py log-level:=INFO data-file:=src/eeg/eeg_simulator/data/random_data.csv sampling-frequency:=500`. If in a different directory, modify the path of data-file accordingly.

3. Check that the data is published by running `ros2 topic echo /eeg/raw_data` in another terminal.

The file `random_data.csv` contains 100 lines of random values between 0 and 1, 64 values per line (i.e., 0.2 seconds of data with the sampling frequency 500 Hz and 64 channels).

The data has been generated with the following Python invocation; modify as needed.

```
import numpy as np
np.savetxt("random_data.csv", 2 * np.random.rand(100, 64) - 1, delimiter=",")
```
