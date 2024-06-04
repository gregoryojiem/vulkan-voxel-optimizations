//
// Created by Gregory on 5/25/2024.
//

#include "TimeManager.h"

std::chrono::time_point<std::chrono::high_resolution_clock> TimeManager::lastTime = std::chrono::high_resolution_clock::now();
float TimeManager::deltaTime;

float TimeManager::setDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> newDeltaTime = currentTime - lastTime;
    lastTime = currentTime;
    deltaTime = newDeltaTime.count();
    return deltaTime;
}

float TimeManager::getDeltaTime() {
    return deltaTime;
}