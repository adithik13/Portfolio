import numpy as np  # numpy handles arrays and mathematical operations. 
import pandas as pd  # pandas allows us to read & process the .csv file easily 
import matplotlib.pyplot as plt  # matplotlib allows us to create the visualization 
from scipy.signal import butter, filtfilt, iirnotch, detrend  # scipy allows us to add the filters for cleaning the signal 

def plot_subplot1(ax, file_path):
    # first we use pandas to read the csv raw emg data 
    data = pd.read_csv(file_path, skiprows=11) # this reads in the file path spefied by main and skips the first 11 rows, which are other info
    data.columns = ['Second', 'CH1 Volt', 'CH2 Volt']  # read in the 3 columns and name them for clarity. we will focus on Ch1 here.

    time = data['Second'].values  # assign "second" col to be time data 
    signal = data['CH2 Volt'].values  # assign "Ch1" col to be signal data 

    # to be safe, we check for NaN or infinite values in the signal which can mess with later calculations
    if np.any(np.isnan(signal)) or np.any(np.isinf(signal)):
        raise ValueError("signal has NaN or infinite vals")

    # remove DC Offset  (shown in matplotlib code as well)
    signal = detrend(signal)

    # since the sample rate was very high (2,500,000), I needed to downsample the data
    downsample_factor = 20  # downsample by 20 
    time_downsampled = time[::downsample_factor]
    signal_downsampled = signal[::downsample_factor]
    fs_downsampled = 2500000 / downsample_factor  

    # Filter 1: Bandpass to focus on emg signal only 
    low_cutoff = 20   # low cutoff
    high_cutoff = 1200  # high cutoff frequency in Hz

    # butterworth bandpass filter : 
    nyquist = fs_downsampled / 2 # nyquist rate is always 2x highest freq or 1/2 sr for this calculation 
    low = low_cutoff / nyquist
    high = high_cutoff / nyquist
    b_bandpass, a_bandpass = butter(2, [low, high], btype='bandpass')  # second order butterworth filter to retain signal shape, and not overdo the smoothing

    # apply the bandpass filter: 
    bandpass_filtered_signal = filtfilt(b_bandpass, a_bandpass, signal_downsampled)

    # Filter 2: Notch filter, focusing on removing 60 Hz noise
    notch_freq = 60  # freq to be removed 
    quality_factor = 20.0  # quality factor, controls the width
    b_notch, a_notch = iirnotch(notch_freq, quality_factor, fs_downsampled)

    # apply the notch filter
    filtered_signal = filtfilt(b_notch, a_notch, bandpass_filtered_signal)

    # check again for NaN or infinite vals
    if np.any(np.isnan(filtered_signal)) or np.any(np.isinf(filtered_signal)):
        raise ValueError("Filtered signal contains NaN or infinite values. Filter configuration may need adjustment.")

    # plot the results
    ax.plot(time_downsampled, filtered_signal, color='black')
    ax.set_xticks([])  # these two lines remove the tick marks 
    ax.set_yticks([])  
    #ax.set_title('CSV SDS00023') # sets the title of the plot, i made it the csv num 
    