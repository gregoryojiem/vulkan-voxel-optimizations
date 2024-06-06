#include "TimeManager.h"

std::chrono::time_point<std::chrono::high_resolution_clock> TimeManager::lastFrameTime = std::chrono::high_resolution_clock::now();
float TimeManager::deltaTime;
std::chrono::time_point<std::chrono::high_resolution_clock> TimeManager::startTime;
bool TimeManager::timerRunning;

float TimeManager::setDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> newDeltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    deltaTime = newDeltaTime.count();
    return deltaTime;
}

float TimeManager::getDeltaTime() {
    return deltaTime;
}

void TimeManager::startTimer() {
    timerRunning = true;
    startTime = std::chrono::high_resolution_clock::now();
}

float TimeManager::finishTimer() {
    if (!timerRunning) {
        return 0.0f;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = endTime - startTime;
    timerRunning = false;
    return elapsed.count();
}