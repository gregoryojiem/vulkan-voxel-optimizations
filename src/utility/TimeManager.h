#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <chrono>
#include <map>
#include <string>

struct TimeProfiler {
    float totalTime;

    void addTime(float time);
    [[nodiscard]] float getTotalTime() const;
};

class TimeManager {
public:
    static float setDeltaTime();
    static float getDeltaTime();

    static void startTimer(const std::string& name);
    static float finishTimer(const std::string& name);

    static void addTimeToProfiler(const std::string& name, float time);
    static float finishProfiler(const std::string& name);
    static void printAllProfiling();

private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime;
    static float deltaTime;
    static std::map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> timers;
    static std::map<std::string, TimeProfiler> profilers;
};

#endif //TIMEMANAGER_H
