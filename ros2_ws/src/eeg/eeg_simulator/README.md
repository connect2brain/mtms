## EEG simulator

### Generating data

The file `random_data.csv` contains 100 lines of random values between 0 and 1, 128 values per line (i.e., 0.2 seconds of data with the sampling frequency 500 Hz, configurable EEG and EMG channels).

The data has been generated with the following Python invocation; modify as needed.

```
import numpy as np
np.savetxt("random_data.csv", 2 * np.random.rand(100, 128) - 1, delimiter=",")
```
