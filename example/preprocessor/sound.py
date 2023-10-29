import multiprocessing
import time

import numpy as np

import preprocessor_bindings

# Override Python's native print() function.
def print(x):
    preprocessor_bindings.log(str(x))


class Preprocessor:
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Load a generic average-head lead field tailored for the NeurOne data:
        self.lfm = np.identity(self.num_of_eeg_channels)
        #self.lfm = np.genfromtxt('leadfield.txt',delimiter='\t')

        # SOUND parameters
        self.iterations = 10
        self.lambda0 = 0.1
        self.learning_rate = 0.01
        self.convergence_boundary = 0.0001
        self.update_interval_in_samples = self.sampling_frequency

        # Initialize state
        self.filter = np.identity(self.num_of_eeg_channels)
        self.updating_filter = False

        self.samples_collected = 0

        self.ongoing_pulse_artifact = False
        self.samples_after_pulse = 0

        # Initialize multiprocessing
        self.pool = multiprocessing.Pool(processes=4)
        self.result = None

        # Configure the length of sample window.
        self.sample_window = [-499, 0]

    def sleep(self, duration_us):
        duration = duration_us / 10 ** 6

        now = time.perf_counter()
        end = now + duration
        while now < end:
            now = time.perf_counter()

    def process(self, time, eeg_data, emg_data, pulse_given):
        self.samples_collected += 1

        if pulse_given:
            self.ongoing_pulse_artifact = True
            self.samples_after_pulse = 0
            print("A pulse was given.")

        # Assuming that an ongoing artifact lasts for 1000 samples; after that, reset the flag.
        if self.ongoing_pulse_artifact:
            self.samples_after_pulse += 1
            if self.samples_after_pulse == 1000:
                self.ongoing_pulse_artifact = False

        if self.result is None and \
           self.samples_collected % self.update_interval_in_samples == 0 and \
           not self.ongoing_pulse_artifact:

            self.result = self.pool.apply_async(sound, (
                eeg_data,
                self.num_of_eeg_channels,
                self.lfm,
                self.iterations,
                self.lambda0,
                self.convergence_boundary,
                self.learning_rate,
            ))

        if self.result is not None:
            try:
                new_filter, new_lambda0 = self.result.get(timeout=0)

                self.filter = new_filter
                self.lambda0 = new_lambda0

                self.result = None

                print("SOUND filter successfully updated.")
                print("New lambda value: {}".format(self.lambda0))

            except multiprocessing.TimeoutError as e:
                pass

        eeg_data_preprocessed = eeg_data[-1,:] @ self.filter.T
        emg_data_preprocessed = emg_data[-1,:]

        return {
            'eeg_data': eeg_data_preprocessed,
            'emg_data': emg_data_preprocessed,
            'valid': not self.ongoing_pulse_artifact,
        }


def sound(eeg_data, num_of_channels, lfm, iterations, lambda0, convergence_boundary, learning_rate):
    # Performs the SOUND algorithm for a given data.

    data = eeg_data.T

    start = time.time()
    print("Updating SOUND filter...")

    n0, _ = data.shape
    data = np.reshape(data, (n0, -1))
    num_of_channels = data.shape[0]
    sigmas = np.ones((num_of_channels, 1))

    LL = lfm @ lfm.T
    dn = np.empty((iterations, 1)) # Empty vector for convergences

    chanPerms = np.zeros((num_of_channels, num_of_channels - 1),dtype=np.int32)

    for i in range(data.shape[0]):
        chanPerms[i, :] = np.setdiff1d(np.arange(1, num_of_channels + 1), i+1) -1

    # Going through all the channels as many times as requested
    dataCov = np.matmul(data, data.T) / data.shape[1]

    # ULTRA_SOUND

    # Johanna's fast Sound

    for k in range(iterations):
        sigmas_old = sigmas

        #print('Performing SOUND. Iteration round:', k+1)

        #Evaluating each channel in a random order
        for i in np.random.permutation(num_of_channels):
            chan = chanPerms[i, :]
            # Defining the whitening operator with the latest noise
            # estimates
            W = np.diagflat(1.0 / sigmas)
            WL =  (1.0 / sigmas[chan]) * lfm[chan, :]
            WLLW =  WL@(WL.T)

            wM = np.zeros(num_of_channels)
            # Calculate the intermediate values for readability
            WL_transpose = np.transpose(WL)
            trace_term = np.trace(WLLW)
            denominator = WLLW + lambda0 * trace_term / (num_of_channels - 1) * np.eye(num_of_channels - 1)
            inv_sigmas = np.diagflat(1.0 / sigmas[chan])

            # Perform the matrix operations step by step
            tmp1 = np.linalg.solve(denominator, inv_sigmas)
            tmp2 = np.matmul(WL_transpose, tmp1)
            wM_chan = np.matmul(lfm[i, :], tmp2)

            # Assign the result to wM[chan]
            wM[chan] = wM_chan

            #wM[chan] = np.matmul(lfm[i, :], np.matmul(WL.T, np.linalg.solve(WLLW + lambda0 * np.trace(WLLW) / (num_of_channels-1) * np.eye(num_of_channels-1), np.diag(1.0 / sigmas[chan]))))
            wM[i] = -1
            sigmas[i] = np.sqrt(np.matmul(np.matmul(wM, dataCov), wM))

        # Following and storing the convergence of the algorithm
        dn[k] = np.max(np.abs(sigmas_old - sigmas) / sigmas_old)
        #print(dn[k])
        if dn[k] < convergence_boundary: # terminates the iteration if the convergence boundary is reached
            break

    # Final data correction based on the final noise-covariance estimate.
    # Calculates matrices needed for SOUND spatial filter (for other functions)
    W = np.diag(1.0 / np.squeeze(sigmas))
    WL = np.matmul(W, lfm)
    WLLW = np.matmul(WL, WL.T)
    C = (WLLW + lambda0 * np.trace(WLLW) / num_of_channels * np.eye(num_of_channels))
    SOUND_Wiener_filter = np.matmul(lfm, np.matmul(WL.T, np.linalg.solve(C, W)))

    #SOUND_Wiener_filter = np.random.rand(62,62) # just for testing that anything is happening

    # Check whether the regularization level is optimal and adjust with the
    # learning rate, when appropriate:

    # find the best-quality channel
    best_ch = np.argmin(sigmas)
    # Calculate the relative error in the best channel caused by SOUND overcorrection:
    rel_err = np.linalg.norm(SOUND_Wiener_filter[best_ch,:]@data - data[best_ch,:])/np.linalg.norm(data[best_ch,:])

    if rel_err > 0.1:
        lambda0 -= learning_rate
#     print("Adjusting lambda value to ", lambda0, "Relative error in clenest channel ", rel_err)
    if rel_err < 0.05:
        lambda0 += learning_rate
#     print("Adjusting lambda value to ", lambda0, "Relative error in clenest channel ", rel_err)

    # TODO: Update lambda0

    end = time.time()
    print("Updating SOUND filter took: {:.1f} ms".format(10 ** 3 * (end-start)))

    return SOUND_Wiener_filter, lambda0
