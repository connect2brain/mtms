
import numpy as np
import mne
from scipy import signal
from . import constants


def get_envelope_data(eegdata, eeg_channel_inds, s_freq):
    """
    1. pick data from the occipital channels
    2. band-pass filter in the alpha frequency range
    3. apply e.g. Hilbert transform and extract amplitude envelope
    """

    # Extract only given (occipital) channels
    occipital_data = eegdata[eeg_channel_inds, :]

    # Band-pass filter data in alpha range with mne filter_data function
    data_filt = mne.filter.filter_data(occipital_data, sfreq=s_freq, l_freq=constants.ALPHA_LOW_FREQ, h_freq=constants.ALPHA_HIGH_FREQ, verbose='CRITICAL')

    # Apply Hilbert transform
    return np.abs(signal.hilbert(data_filt, axis=1))


def get_alpha_estimate(eegdata, eeg_channel_inds, s_freq):
    """
    1. pick data from the occipital channels
    2. band-pass filter in the alpha frequency range
    3. apply e.g. Hilbert transform and extract amplitude envelope
    4. average envelope values
    """
    return np.mean(get_envelope_data(eegdata, eeg_channel_inds, s_freq), axis=(0,1))


def compute_baseline_stats(alpha_estimates):
    """
    call get_alpha_estimate for the data, collected during the baseline block
    compute the mean and the standard deviation of the alpha amplitude values
    during the whole baseline block
    """
    return np.mean(alpha_estimates), np.std(alpha_estimates)


def scale_alpha_estimate_to_step(current_alpha, baseline_mean, baseline_std):
    """
    1. scale the current alpha amplitude value by subtracting the mean and
    dividing by the standard deviation of the alpha values during the baseline
    block
    """
    scaled_alpha = (current_alpha - baseline_mean) / baseline_std
    return scaled_alpha
