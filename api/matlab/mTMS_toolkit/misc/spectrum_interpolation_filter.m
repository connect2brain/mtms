function [filtered_signal] = spectrum_interpolation_filter(signal, sampling_rate, line_freq, varargin)
% SPECTRUM_INTERPOLATION_FILTER Removes power line noise using spectrum interpolation.
%
% This function is a clean-room implementation based on the method described in:
% Leske, S., & Dalal, S. S. (2019). Reducing power line noise in EEG and
% MEG data via spectrum interpolation. NeuroImage, 189, 460-473.
% https://doi.org/10.1016/j.neuroimage.2019.01.026
%
% Usage:
%   filtered_signal = spectrum_interpolation_filter(signal, sampling_rate, line_freq)
%   filtered_signal = spectrum_interpolation_filter(signal, sampling_rate, line_freq, 'Bandwidth', 1, 'NeighbourWidth', 2)
%
% Inputs:
%   signal          - Input data matrix (Channels x Time).
%   sampling_rate   - The sampling frequency of the signal in Hz.
%   line_freq       - The frequency of the power line noise to remove (e.g., 50 or 60 Hz).
%                     Can be a vector for multiple frequencies (e.g., [50 100 150]).
%
% Optional Key-Value Pair Arguments:
%   'Bandwidth'         - The half-bandwidth of the frequency band to be interpolated (in Hz).
%                         Default is 1 Hz, meaning a total bandwidth of 2 Hz around the line_freq.
%   'NeighbourWidth'    - The width of the neighboring frequency bands used for amplitude
%                         estimation (in Hz). Default is 2 Hz.
%
% Output:
%   filtered_signal - The signal with the power line noise removed.

% --- Input Parser for Optional Arguments ---
p = inputParser;
addParameter(p, 'Bandwidth', 1, @isnumeric);
addParameter(p, 'NeighbourWidth', 2, @isnumeric);
parse(p, varargin{:});

bandwidth = p.Results.Bandwidth;
neighbour_width = p.Results.NeighbourWidth;

% --- Data and Parameter Setup ---
[num_channels, num_samples] = size(signal);
line_freq = line_freq(:); % Ensure it's a column vector

% --- Recursive Call for Multiple Frequencies ---
% If multiple frequencies are specified, filter them sequentially.
if numel(line_freq) > 1
    filtered_signal = signal;
    for i = 1:numel(line_freq)
        % Ensure bandwidth and neighbour_width arrays match line_freq array length
        bw = bandwidth(min(i, end));
        nw = neighbour_width(min(i, end));
        filtered_signal = spectrum_interpolation_filter(filtered_signal, sampling_rate, line_freq(i), 'Bandwidth', bw, 'NeighbourWidth', nw);
    end
    return;
end

% --- Core Filtering Logic ---

% For robust estimation, use the largest possible segment of the data that
% contains an integer number of line frequency cycles.
num_cycles_in_data = floor(num_samples / sampling_rate * line_freq);
if num_cycles_in_data == 0
    warning('Data segment is too short to contain a full cycle of the line noise. Skipping filter.');
    filtered_signal = signal;
    return;
end
estimation_samples = round(num_cycles_in_data * sampling_rate / line_freq);
estimation_indices = 1:estimation_samples;

% 1. Detrend: Remove the mean to prevent spectral leakage from a DC offset.
mean_signal = mean(signal(:, estimation_indices), 2);
detrended_signal = bsxfun(@minus, signal, mean_signal);

% 2. Define Frequency Bins for DFT
% The frequency resolution (Rayleigh frequency) depends on the length of the estimation window.
freq_resolution = sampling_rate / estimation_samples;

% Determine the number of frequency bins to analyze on each side of the line frequency.
total_half_width = bandwidth + neighbour_width;
num_bins_side = round(total_half_width / freq_resolution);

% Create a vector of frequency bins centered around the line frequency.
freq_bins = line_freq + (-num_bins_side:num_bins_side) * freq_resolution;

% 3. Estimate Complex Amplitudes (DFT)
% Create the basis functions (complex exponentials) for the DFT.
time_vector = (0:num_samples-1) / sampling_rate;
dft_basis = exp(1i * 2 * pi * freq_bins(:) * time_vector);

% Project the data onto the basis functions to get the complex amplitudes.
% This is done only on the integer-cycle portion of the data for accuracy.
complex_amplitudes = (2 / estimation_samples) * detrended_signal(:, estimation_indices) * dft_basis(:, estimation_indices)';

% 4. Identify Stop-Band and Neighbour-Band Bins
% Find the indices of the frequency bins that fall within the stop-band.
stop_band_indices = find(abs(freq_bins - line_freq) <= bandwidth);

% Create a logical mask to identify the bins to be replaced.
is_stop_band = false(1, length(freq_bins));
is_stop_band(stop_band_indices) = true;

% 5. Interpolate the Spectrum
% The signal within the stop-band needs to be removed.
signal_to_remove = real(complex_amplitudes(:, is_stop_band) * dft_basis(is_stop_band, :));

% Estimate the new amplitude from the average of the neighbouring bands.
% The neighbours are all bins that are NOT in the stop-band.
mean_neighbour_amplitude = mean(abs(complex_amplitudes(:, ~is_stop_band)), 2);

% Create the replacement amplitudes. The phase is preserved from the original
% signal to avoid phase distortion.
original_phase = angle(complex_amplitudes(:, is_stop_band));
replacement_amplitudes = mean_neighbour_amplitude .* exp(1i * original_phase);

% Create the replacement signal.
signal_to_add = real(replacement_amplitudes * dft_basis(is_stop_band, :));

% 6. Reconstruct the Signal
% Subtract the original noise component and add the interpolated component.
filtered_signal = detrended_signal - signal_to_remove + signal_to_add;

% 7. Add the mean back to the signal.
filtered_signal = bsxfun(@plus, filtered_signal, mean_signal);

end
