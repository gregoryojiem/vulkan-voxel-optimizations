#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <chrono>

class TimeManager {
public:
    TimeManager() : lastTime(std::chrono::high_resolution_clock::now()) {}

    float getDeltaTime();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

#endif //TIMEMANAGER_H
