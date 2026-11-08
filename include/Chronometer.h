#pragma once
#include <chrono>
#include <vector>

// Sample N times duration and give average 
class Chronometer
{
    int sampleCount{};
    int timeIterator{};
	std::chrono::steady_clock::time_point _start{}, _finish{};
    std::vector<double> timeSteps{};

public:
    Chronometer(int sampleCount = 100);
    void start();
    double stop();
    void reset();

};

