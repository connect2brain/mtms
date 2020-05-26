#!/usr/bin/env python3

import os
import sys
import argparse
import pathlib
try:
    import numpy as np
except ImportError as e:
    sys.stderr.write("Numpy not found. Install with 'pip install numpy'.\n")
    sys.exit(1)
try:
    import pandas as pd
except ImportError as e:
    sys.stderr.write("Pandas not found. Install with 'pip install pandas'.\n")
    sys.exit(1)
try:
    import matplotlib as mpl
    import matplotlib.pyplot as plt
except ImportError as e:
    sys.stderr.write("Matplotlib not found. Install with 'pip install matplotlib'.\n")
    sys.exit(1)
try:
    import seaborn as sns
    sns.set_style('whitegrid')
except ImportError as e:
    sys.stderr.write("Seaborn not found. Install with 'pip install seaborn'.\n")
    sys.exit(1)


def plot_data_from_file(path, data_interval=0.00625, start_time=0.0):
    """Plots a series of charts from the given data file.

    Arguments
    ---------
    path : pathlib.Path
        Path instance to a proper CSV file with timings on columns 0, 1, and 3.
    data_interval : float
        Interval (in s) of each data point.
    start_time : float
        Start time of time series.
    """
    with mpl.rc_context(rc={
            'interactive': True,
            'lines.linewidth': 1,
            'font.family': 'sans-serif',
            'font.weight': 'normal',
            'font.size': '8'
            }):
        rows = 2
        cols = 2
        fig, ax = plt.subplots(rows, cols, figsize=(5*cols, 2.5*rows))

        # Load data
        d = pd.read_csv(path, names=('tick', 'send', 'recv'))
        d['t'] = np.arange(start_time, start_time+data_interval*len(d), data_interval)
        # Correct timings
        d['recv'] = d['recv'] - d['tick'].min()
        d['send'] = d['send'] - d['tick'].min()
        d['tick'] = d['tick'] - d['tick'].min()
        # Calculate new variables
        d['tick_to_send'] = d['send'] - d['tick']
        d['send_to_recv'] = d['recv'] - d['send']

        # Plot 1: Tick to send and send to recv vs time
        ax[0,0].plot(d['t'], 1000*d['tick_to_send'], 'b-')
        ax[0,0].plot(d['t'], 1000*(d['tick_to_send']+d['send_to_recv']), 'g-')
        ax[0,0].set_xlabel("Time, s")
        ax[0,0].set_ylabel("Delay from tick out, ms")
        ax[0,0].set_title("Send and recv delays")

        # Plot 2: Zoom in at the start
        ax[1,0].plot(d['t'], 1000*d['tick_to_send'], 'b-')
        ax[1,0].plot(d['t'], 1000*(d['tick_to_send']+d['send_to_recv']), 'g-')
        ax[1,0].set_xlabel("Time, s")
        ax[1,0].set_ylabel("Delay from tick out, ms")
        ax[1,0].set_title("Send and recv delays (zoom to start)")
        ax[1,0].set_xlim(0, 0.1*d['t'].max())

        # Plot 3: Zoom at the end
        ax[1,1].plot(d['t'], 1000*d['tick_to_send'], 'b-')
        ax[1,1].plot(d['t'], 1000*(d['tick_to_send']+d['send_to_recv']), 'g-')
        ax[1,1].set_xlabel("Time, s")
        ax[1,1].set_ylabel("Delay from tick out, ms")
        ax[1,1].set_title("Send and recv delays (zoom to end)")
        ax[1,1].set_xlim(0.9*d['t'].max(), d['t'].max())

        # Plot 4: Histogram
        ax[0,1].hist(1000*d['tick_to_send'], bins=25, color='b', alpha=0.5, density=1)
        ax[0,1].hist(1000*d['send_to_recv'], bins=25, color='g', alpha=0.5, density=1)
        ax[0,1].set_xlabel("Delay, ms")
        ax[0,1].set_ylabel("Normalized density")
        ax[0,1].set_title("Delay to send (from tick) and receive (from send)")

        # Plot 4: Kernel density
        sns.kdeplot(1000*d['tick_to_send'], ax=ax[0,1], color='b', alpha=0.75)
        sns.kdeplot(1000*d['send_to_recv'], ax=ax[0,1], color='g', alpha=0.75)

        # Plot 3: 

        plt.tight_layout()
        plt.show()
        plt.waitforbuttonpress()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('csvfile', help='Input CSV file')
    args = parser.parse_args()

    csvfile = pathlib.Path(args.csvfile)
    if not csvfile.is_file():
        sys.stderr.write("CSV file does not exist.\n")
        sys.exit(1)

    plot_data_from_file(csvfile)
