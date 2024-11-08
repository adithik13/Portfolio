import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.signal import butter, filtfilt, iirnotch, detrend

def plot_subplot2(ax, file_path):
    data = pd.read_csv(file_path, skiprows=11)
    data.columns = ['Second', 'CH1 Volt', 'CH2 Volt']

    time = data['Second'].values
    signal = data['CH1 Volt'].values

    if np.any(np.isnan(signal)) or np.any(np.isinf(signal)):
        raise ValueError("Original signal contains NaN or infinite values. Please check your data.")

    signal = detrend(signal)

    downsample_factor = 20
    time_downsampled = time[::downsample_factor]
    signal_downsampled = signal[::downsample_factor]
    fs_downsampled = 2500000 / downsample_factor

    low_cutoff = 20
    high_cutoff = 1200

    nyquist = fs_downsampled / 2
    low = low_cutoff / nyquist
    high = high_cutoff / nyquist
    b_bandpass, a_bandpass = butter(2, [low, high], btype='bandpass')

    bandpass_filtered_signal = filtfilt(b_bandpass, a_bandpass, signal_downsampled)

    notch_freq = 60
    quality_factor = 20.0
    b_notch, a_notch = iirnotch(notch_freq, quality_factor, fs_downsampled)

    filtered_signal = filtfilt(b_notch, a_notch, bandpass_filtered_signal)

    if np.any(np.isnan(filtered_signal)) or np.any(np.isinf(filtered_signal)):
        raise ValueError("Filtered signal contains NaN or infinite values. Filter configuration may need adjustment.")

    ax.plot(time_downsampled, filtered_signal, color='black')
    ax.set_xticks([])
    ax.set_yticks([])
    #ax.set_title('CSV SDS00012')
