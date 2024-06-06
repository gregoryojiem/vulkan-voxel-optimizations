#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <chrono>

class TimeManager {
public:
    static float setDeltaTime();
    static float getDeltaTime();

    static void startTimer();
    static float finishTimer();

private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime;
    static float deltaTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    static bool timerRunning;
};

#endif //TIMEMANAGER_H
