#include "PreambleDetector.h"
#include <cmath>
#include <iostream>
#include <numeric>
#include <cstdint>
#include <algorithm>
#include <chrono>

vector<float> buffer(BUFFER_SIZE * 10, 0.0f);
mutex mutex_preamble;
condition_variable cond;
vector<float> preambleTemplate;


vector<float> generateBFSKPreambleTemplate() {
	vector<int16_t> bfskWave;
	const vector<char> preambleBits = { '0', '0', '1', '1', '0', '0', '1', '1' };

	for (char bit : preambleBits) {
		double frequency = (bit == '1') ? 50000.0 : 47000.0;
		//double frequency = (bit == '1') ? 9000.0 : 7000.0;
		auto wave = generateSineWave(frequency, BUFFER_SIZE);
		bfskWave.insert(bfskWave.end(), wave.begin(), wave.end());
	}

	vector<float> templateFloat;
	for (int16_t sample : bfskWave) {
		templateFloat.push_back(static_cast<float>(sample) / AMPLITUDE);  // Normalize to [-1, 1]
	}

	return templateFloat;
}

vector<int16_t> generateSineWave(double frequency, int samples) {
	vector<int16_t> wave(samples);
	for (int i = 0; i < samples; ++i) {
		wave[i] = static_cast<int16_t>(AMPLITUDE * sin(2 * PI * frequency * i / SAMPLE_RATE));
	}
	return wave;
}

void updateBuffer(const vector<float>& newData) {
	lock_guard<mutex> lock(mutex_preamble);

	// Sliding window: discard the oldest 192 samples and append 192 new samples
	buffer.erase(buffer.begin(), buffer.begin() + BUFFER_SIZE);
	buffer.insert(buffer.end(), newData.begin(), newData.end());

	cond.notify_one();  // Wake up the run()
}

void run() {
	preambleTemplate = generateBFSKPreambleTemplate();
	while (true) {
		{
			unique_lock<mutex> lock(mutex_preamble);
			cond.wait(lock);
		}
		detectPreamble();
	}
}

void detectPreamble() {

	if (crossCorrelation(buffer, preambleTemplate)) {
		cout << "[PreambleDetector] Preamble detected!" << endl;
		exit(0);
	}
}

bool crossCorrelation(const vector<float>& data, const vector<float>& templateData) {
	if (data.size() != 1920 || templateData.size() != 1536) {
		cerr << "Error: Input vectors have incorrect sizes. Expected data.size()=1920 and templateData.size()=1536." << endl;
		return false;
	}

	//auto start_time = chrono::high_resolution_clock::now();
	// The maximum possible lag/shift position
	const size_t maxLag = data.size() - templateData.size();

	// Vector to store correlation values for each possible starting position
	vector<float> correlationValues(maxLag + 1, 0.0f);

	// Calculate the energy of the template for normalization
	float templateEnergy = 0.0f;
	for (const auto& val : templateData) {
		templateEnergy += val * val;
	}

	// Calculate cross-correlation for each possible starting position
	for (size_t lag = 0; lag <= maxLag; ++lag) {
		float correlation = 0.0f;
		float dataEnergy = 0.0f;

		for (size_t i = 0; i < templateData.size(); ++i) {
			correlation += data[lag + i] * templateData[i];
			dataEnergy += data[lag + i] * data[lag + i];
		}

		// Normalize the correlation value if possible
		if (dataEnergy > 0 && templateEnergy > 0) {
			correlationValues[lag] = correlation / sqrt(dataEnergy * templateEnergy);
		}
		else {
			correlationValues[lag] = 0.0f;
		}
	}

	// Find the starting position with the maximum correlation
	size_t bestPosition = 0;
	float maxCorrelation = correlationValues[0];

	for (size_t i = 1; i <= maxLag; ++i) {
		if (correlationValues[i] > maxCorrelation) {
			maxCorrelation = correlationValues[i];
			bestPosition = i;
		}
	}

	//auto end_time = chrono::high_resolution_clock::now();
	//auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
	//cout << "Time taken: " << duration.count() << " microseconds" << endl;

	// Check if the maximum correlation exceeds the threshold
	bool detectionSuccessful = (maxCorrelation >= DETECTION_THRESHOLD);

	if (detectionSuccessful) {
		cout << "Template detected! Correlation value exceeds threshold." << endl;
		// Output the best starting position to the console
		cout << "Most likely starting position: " << bestPosition << endl;
		cout << "Maximum correlation value: " << maxCorrelation << endl;
	}
	else {
		//cout << "Template not detected. Correlation value below threshold." << endl;
	}

	return detectionSuccessful;
}
