#include "SamplingThread.h"
#include <iostream>
#include "PreambleDetector.h"
extern PreambleDetector preambleDetector;

constexpr float INT24_MAX = 8388608.0f;
constexpr int BUFFER_SIZE = 192;
constexpr int CHANNEL_COUNT = 4;

void samplingThreadFunc(snd_pcm_t* pcm_handle) {
    int32_t rawSamples[BUFFER_SIZE * CHANNEL_COUNT];

    while (true) {
        snd_pcm_sframes_t frames = snd_pcm_readi(pcm_handle, rawSamples, BUFFER_SIZE);
        if (frames < 0) {
            snd_pcm_prepare(pcm_handle);
            continue;
        }

        std::vector<float> frameData;
        frameData.reserve(BUFFER_SIZE);

        for (int i = 0; i < frames; ++i) {
            int32_t sample = rawSamples[i * CHANNEL_COUNT ];  // Take the first channel
            float normalized = (sample >> 8) / INT24_MAX;
            frameData.push_back(normalized);
        }
        // --- Notify preambleDetector to update the sliding window ---
        preambleDetector.updateBuffer(frameData);
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (taskQueue.size() > 50) {
                taskQueue.pop();
            }
            taskQueue.push(std::move(frameData));
        }
        queueCond.notify_one();
    }
}
