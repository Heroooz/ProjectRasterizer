#pragma once
#include <chrono>

class Time {
public:
    Time();
    void Update();
    float GetDeltaTime() const;

private:
    std::chrono::steady_clock::time_point lastTime;
    float deltaTime;
};