from concurrent.futures import ThreadPoolExecutor
import time

import numpy as np

import preprocessor_bindings

def log(x):
    preprocessor_bindings.log(str(x))


class Preprocessor:
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Loads a generic average-head lead field tailored for the NeurOne data:
        self.LFM = np.identity(self.num_of_eeg_channels)
        #self.LFM = np.genfromtxt('leadfield.txt',delimiter='\t')

        # SOUND parameters
        self.num_of_sound_iterations = 10
        self.lambda0 = 0.1
        self.lambda_learning_rate = 0.01
        self.conv_bound = 0.0001
        self.update_interval = self.sampling_frequency

        # Initialize state
        self.sound_filter = np.identity(self.num_of_eeg_channels)

        self.samples_collected = 0
        self.ongoing_tms_artifact = False
        self.updating = False

        self.executor = ThreadPoolExecutor()  # Initialize the executor

        # Configure the length of sample window.
        self.sample_window = [-499, 0]

    def sleep(self, duration_us):
        duration = duration_us / 10 ** 6

        now = time.perf_counter()
        end = now + duration
        while now < end:
            now = time.perf_counter()

    def identify_tms(self, eeg_data):
        mean_abs_diff = np.abs(np.diff(eeg_data[-2:,20]))
        tms = mean_abs_diff > 1000

        return tms

    def process(self, time, eeg_data, emg_data):
        self.samples_collected += 1

        # Assuming that an ongoing artifact lasts for 1000 samples; after that, reset the flag.
        if self.ongoing_tms_artifact:
            self.samples_after_tms += 1
            if self.samples_after_tms == 1000:
                self.ongoing_tms_artifact = False

        # Every 2 samples, try to identify a TMS pulse if not already identified.
        if self.samples_collected % 2 == 0 and not self.ongoing_tms_artifact and self.identify_tms(eeg_data):
            self.ongoing_tms_artifact = True
            self.samples_after_tms = 0

        if not self.updating and self.samples_collected % self.update_interval == 0 and not self.ongoing_tms_artifact:
            self.updating = True

            self.sound(eeg_data)
            self.updating = False
            # future = self.executor.submit(self.SOUND, eeg_data)

            # def callback(future):
            #     self.sound_filter = future.result()
            #     self.updating = False

            # future.add_done_callback(callback)

        return {
            'eeg_data': [],
            'emg_data': [],
            'valid': True,
        }

    def sound(self, eeg_data):
        # Performs the SOUND algorithm for a given data.

        data = eeg_data.T

        start = time.time()
        log("Starting SOUND update")

        n0, _ = data.shape
        data = np.reshape(data, (n0, -1))
        chanN = data.shape[0]
        sigmas = np.ones((self.num_of_eeg_channels, 1))

        LL = self.LFM @ self.LFM.T
        dn = np.empty((self.num_of_sound_iterations, 1)) # Empty vector for convergences

        chanPerms = np.zeros((self.num_of_eeg_channels, self.num_of_eeg_channels - 1),dtype=np.int32)

        for i in range(data.shape[0]):
            chanPerms[i, :] = np.setdiff1d(np.arange(1, self.num_of_eeg_channels + 1), i+1) -1

        # Going through all the channels as many times as requested
        dataCov = np.matmul(data, data.T) / data.shape[1]

        # ULTRA_SOUND

        # Johanna's fast Sound

        for k in range(self.num_of_sound_iterations):
            sigmas_old = sigmas

            #print('Performing SOUND. Iteration round:', k+1)

            #Evaluating each channel in a random order
            for i in np.random.permutation(chanN):
                chan = chanPerms[i, :]
                # Defining the whitening operator with the latest noise
                # estimates
                W = np.diagflat(1.0 / sigmas)
                WL =  (1.0 / sigmas[chan]) * self.LFM[chan, :]
                WLLW =  WL@(WL.T)

                wM = np.zeros(chanN)
                # Calculate the intermediate values for readability
                WL_transpose = np.transpose(WL)
                trace_term = np.trace(WLLW)
                denominator = WLLW + self.lambda0 * trace_term / (chanN - 1) * np.eye(chanN - 1)
                inv_sigmas = np.diagflat(1.0 / sigmas[chan])

                # Perform the matrix operations step by step
                tmp1 = np.linalg.solve(denominator, inv_sigmas)
                tmp2 = np.matmul(WL_transpose, tmp1)
                wM_chan = np.matmul(self.LFM[i, :], tmp2)

                # Assign the result to wM[chan]
                wM[chan] = wM_chan

                #wM[chan] = np.matmul(self.LFM[i, :], np.matmul(WL.T, np.linalg.solve(WLLW + self.lambda0 * np.trace(WLLW) / (chanN-1) * np.eye(chanN-1), np.diag(1.0 / sigmas[chan]))))
                wM[i] = -1
                sigmas[i] = np.sqrt(np.matmul(np.matmul(wM, dataCov), wM))

            # Following and storing the convergence of the algorithm
            dn[k] = np.max(np.abs(sigmas_old - sigmas) / sigmas_old)
            #print(dn[k])
            if dn[k] < self.conv_bound: # terminates the iteration if the convergence boundary is reached
                break

        # Final data correction based on the final noise-covariance estimate.
        # Calculates matrices needed for SOUND spatial filter (for other functions)
        W = np.diag(1.0 / np.squeeze(sigmas))
        WL = np.matmul(W, self.LFM)
        WLLW = np.matmul(WL, WL.T)
        C = (WLLW + self.lambda0 * np.trace(WLLW) / chanN * np.eye(chanN))
        SOUND_Wiener_filter = np.matmul(self.LFM, np.matmul(WL.T, np.linalg.solve(C, W)))

        #SOUND_Wiener_filter = np.random.rand(62,62) # just for testing that anything is happening

        # Check whether the regularization level is optimal and adjust with the
        # learning rate, when appropriate:

        # find the best-quality channel
        best_ch = np.argmin(sigmas)
        # Calculate the relative error in the best channel caused by SOUND overcorrection:
        rel_err = np.linalg.norm(SOUND_Wiener_filter[best_ch,:]@data - data[best_ch,:])/np.linalg.norm(data[best_ch,:])

        if rel_err > 0.1:
            self.lambda0 -= self.lambda_learning_rate
    #     print("Adjusting lambda value to ", self.lambda0, "Relative error in clenest channel ", rel_err)
        if rel_err < 0.05:
            self.lambda0 += self.lambda_learning_rate
    #     print("Adjusting lambda value to ", self.lambda0, "Relative error in clenest channel ", rel_err)

        end = time.time()
        log("Finished SOUND update")
        log(end-start)

        return SOUND_Wiener_filter
