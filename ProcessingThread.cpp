#include "ProcessingThread.h"

extern std::queue<std::vector<float>> taskQueue;
extern std::mutex queueMutex;
extern std::condition_variable queueCond;

constexpr int BUFFER_SIZE = 192;
constexpr int SAMPLE_RATE = 192000;

void processingThreadFunc() {
    while (true) {
        std::vector<float> frameData;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, [] { return !taskQueue.empty(); });
            frameData = std::move(taskQueue.front());
            taskQueue.pop();
        }

        float fftSamples[BUFFER_SIZE];
        std::copy(frameData.begin(), frameData.end(), fftSamples);

        float peakFreq = FFT::computePeakFrequency(fftSamples, BUFFER_SIZE, SAMPLE_RATE);
        std::cout << "Peak Frequency: " << std::fixed << std::setprecision(2) << peakFreq << " Hz" << std::endl;
    }
}