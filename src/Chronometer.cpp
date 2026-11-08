#include "Chronometer.h"
#include <iostream>
#include <execution>

Chronometer::Chronometer(int sampleCount) : sampleCount{ sampleCount } {
    timeSteps = std::vector<double>(sampleCount, 0);
}

void Chronometer::start() {
    _start = std::chrono::steady_clock::now();
}

double Chronometer::stop() {
    _finish = std::chrono::steady_clock::now();

    double elapsed_time = std::chrono::duration_cast<
        std::chrono::duration<double>>(_finish - _start).count();

    timeSteps[timeIterator++] = elapsed_time;

    if (timeIterator >= sampleCount) {
        timeIterator = 0;
        return std::accumulate(timeSteps.begin(), timeSteps.end(), 0.0) / 60;
    }
    return 0.0;
}

void Chronometer::reset() {
    timeIterator = 0;
    timeSteps = std::vector<double>(sampleCount, 0);
}