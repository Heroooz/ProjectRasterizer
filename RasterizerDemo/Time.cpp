#include "Time.h"

Time::Time() : lastTime(std::chrono::high_resolution_clock::now()), deltaTime(0.0f) {}

void Time::Update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = currentTime - lastTime;
    deltaTime = delta.count();
    lastTime = currentTime;
}

float Time::GetDeltaTime() const {
    return deltaTime;
}