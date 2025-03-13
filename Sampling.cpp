#include "Sampling.h"
#include "PreambleDetector.h"

void samplingThreadFunc(snd_pcm_t* pcm_handle)
{
	int32_t rawSamples[BUFFER_SIZE_SAMPLING * CHANNEL_COUNT];

	while (true) {
		snd_pcm_sframes_t frames = snd_pcm_readi(pcm_handle, rawSamples, BUFFER_SIZE_SAMPLING);

		if (frames < 0) {
			snd_pcm_prepare(pcm_handle);
			continue;
		}

		vector<float> frameData;
		frameData.reserve(BUFFER_SIZE_SAMPLING);

		for (int i = 0; i < frames; ++i) {
			int32_t sample = rawSamples[i * CHANNEL_COUNT];  // Take the first channel
			//int32_t sample = rawSamples[i * CHANNEL_COUNT + 2];  // Take the Loopback channel
			float normalized = (static_cast<float>(sample) / 256) / INT24_MAX;
			frameData.push_back(normalized);
		}

		float fftSamples[BUFFER_SIZE];
		std::copy(frameData.begin(), frameData.end(), fftSamples);

		float peakFreq = FFT::computePeakFrequency(fftSamples, BUFFER_SIZE, SAMPLE_RATE);
		std::cout << "Peak Frequency: " << peakFreq << " Hz" << std::endl;

		// --- Notify preambleDetector to update the sliding window ---
		updateBuffer(frameData);
	}
}
