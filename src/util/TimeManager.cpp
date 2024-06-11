#include "TimeManager.h"

#include <iostream>
#include <vector>

std::chrono::time_point<std::chrono::high_resolution_clock> TimeManager::lastFrameTime =
    std::chrono::high_resolution_clock::now();
float TimeManager::deltaTime;

std::map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> TimeManager::timers;
std::map<std::string, TimeProfiler> TimeManager::profilers;

std::vector<float> TimeManager::frameTimes;
float TimeManager::timeBetweenDisplay = 0.25f;
float TimeManager::accumulatedTime = 0;

void TimeProfiler::addTime(const float time) {
    totalTime += time;
}

float TimeProfiler::getTotalTime() const {
    return totalTime;
}

float TimeManager::setDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> newDeltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    deltaTime = newDeltaTime.count();
    return deltaTime;
}

float TimeManager::getDeltaTime() {
    return deltaTime;
}

void TimeManager::startTimer(const std::string& name) {
    timers[name] = std::chrono::high_resolution_clock::now();
}

float TimeManager::finishTimer(const std::string& name) {
    if (!timers.contains(name)) {
        std::cerr << name << " timer could not be finished because it was not found!\n";
        return -1;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = endTime - timers[name];
    return elapsed.count();
}

void TimeManager::addTimeToProfiler(const std::string& name, const float time) {
    if (!profilers.contains(name)) {
        profilers[name] = TimeProfiler{};
    }
    profilers[name].addTime(time);
}

float TimeManager::finishProfiler(const std::string& name) {
    if (!profilers.contains(name)) {
        return 0.0f;
    }
    float profilerTime = profilers[name].getTotalTime();
    profilers.erase(name);
    return profilerTime;
}

void TimeManager::printAllProfiling() {
    std::cout << "Profiling results:\n";

    std::vector<std::pair<std::string, TimeProfiler>> sortedProfilers(profilers.begin(), profilers.end());
    std::sort(sortedProfilers.begin(), sortedProfilers.end(),
              [](const std::pair<std::string, TimeProfiler>& a, const std::pair<std::string, TimeProfiler>& b) {
                  return a.second.getTotalTime() > b.second.getTotalTime();
              });

    float totalTime = 0;
    for (auto&[profilerName, profiler] : sortedProfilers) {
        std::cout << profilerName << " took a total of " << profiler.getTotalTime() << " seconds\n";
        totalTime += profiler.getTotalTime();
    }
    std::cout << "Total time: " << totalTime << " seconds\n";
}

float TimeManager::queryFPS() {
    const float deltaTime = getDeltaTime();
    frameTimes.push_back(deltaTime);
    accumulatedTime += deltaTime;

    if (accumulatedTime < timeBetweenDisplay) {
        return -1.0f;
    }

    accumulatedTime = 0;
    float totalTime = 0;
    for (const auto &time: frameTimes) {
        totalTime += time;
    }

    const float fps = static_cast<float>(frameTimes.size()) / totalTime;
    frameTimes.clear();
    accumulatedTime = 0;
    return fps;
}