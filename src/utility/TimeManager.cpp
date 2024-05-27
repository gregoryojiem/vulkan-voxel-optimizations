//
// Created by Gregory on 5/25/2024.
//

#include "TimeManager.h"

float TimeManager::getDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    return deltaTime.count();
}