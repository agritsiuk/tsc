#include <gtest/gtest.h>

#include <Tsc.hpp>
#include <iostream>
#include <sstream>
#include <thread>

using namespace tsc;

TEST(TscTest, outputMeasurements) {
    Tsc tsc;
    std::cout << std::fixed;
    std::cout << "Rdtsc cost in cycles: " << tsc.rdtscCyclesCost() << std::endl;
    std::cout << "Real clock cost in cycles: " << tsc.realClockCyclesCost() << std::endl;
    std::cout << "CPU clock rate: " << tsc.cpuClockRate() << std::endl;

    const auto start = tsc.now();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    const auto finish = tsc.now();
    std::cout << "Sleep duration in nanos: " << tsc.toNanos(tsc.duration(start, finish)).count() << std::endl;
}
