# Preamble_Detection

## Detailed File Descriptions

1. SamplingThread (SamplingThread.h/.cpp)

- Function: Real-time audio data acquisition using ALSA.

- Implementation: Continuously reads audio data from the ALSA interface, normalizes it, maintains a buffer, and notifies other threads.

2. ProcessingThread (ProcessingThread.cpp/.h)

- Function: Fetches audio data from a queue and performs FFT spectrum analysis.

- Implementation: Uses FFT algorithms to analyze frequency spectrum, outputting the main frequency in real-time.

3. AudioDevice (AudioDevice.h/.cpp)

- Function: Initializes, configures, and manages the ALSA audio device.

- Implementation: Sets parameters such as sampling rate (192 kHz), channel count, and data format for audio input.

4. FFT Utility (fft.cpp/.h)

- Function: Provides FFT (Fast Fourier Transform) algorithm implementation and peak frequency detection.

- Implementation: Implements the recursive Cooley-Tukey FFT algorithm for spectral analysis, identifying the frequency with the highest energy.

4. PreambleDetector (PreambleDetector.h/.cpp)

- Function: Detects specific BFSK (Binary Frequency-Shift Keying) preamble sequences within audio data.

- Implementation: Utilizes FFT-based cross-correlation for rapid, accurate detection of the preamble (001100, with 10 kHz representing '0' and 12 kHz representing '1').
