#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <chrono>

class TimeManager {
public:
    static float setDeltaTime();
    static float getDeltaTime();

private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    static float deltaTime;
};

#endif //TIMEMANAGER_H
