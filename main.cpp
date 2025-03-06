#include "AudioDevice.h"
#include "SamplingThread.h"
#include "ProcessingThread.h"
#include <thread>
#include "PreambleDetector.h"


std::queue<std::vector<float>> taskQueue;
std::mutex queueMutex;
std::condition_variable queueCond;
PreambleDetector preambleDetector;

int main() {
    snd_pcm_t* pcm_handle = nullptr;
    if (!AudioDevice::init(pcm_handle)) {
        return -1;
    }

    std::thread samplingThread(samplingThreadFunc, pcm_handle);
    std::thread processingThread(processingThreadFunc);
    std::thread preambleThread(&PreambleDetector::run, &preambleDetector);

    samplingThread.join();
    processingThread.join();
    preambleThread.join();

    AudioDevice::close(pcm_handle);
    return 0;
}
