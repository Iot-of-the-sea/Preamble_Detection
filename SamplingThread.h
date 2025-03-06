#pragma once
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <alsa/asoundlib.h>

extern std::queue<std::vector<float>> taskQueue;
extern std::mutex queueMutex;
extern std::condition_variable queueCond;

void samplingThreadFunc(snd_pcm_t* pcm_handle);

