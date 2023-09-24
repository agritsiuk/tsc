/**
 * Автор Андрей Грицюк, Москва 2023.
 * Библиотека для преобразования счетчика циклов процессора в тип системного времени.
 * https://github.com/agritsiuk/tsc
 */
#include <x86intrin.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

namespace tsc {

struct TscTraits {
    static constexpr std::size_t kCostMeasurements{101};
    static constexpr std::size_t kRateMeasurements{11};
    static constexpr std::chrono::milliseconds kRateMeasureDuration{20};
};

template <class Traits>
class TscImpl;

using Tsc = TscImpl<TscTraits>;

template <class Traits>
class TscImpl {
   public:
    using TscTimepoint = std::uint64_t;
    using TscDuration = std::int64_t;

   public:
    TscImpl();

    TscTimepoint now();
    TscDuration duration(TscTimepoint start, TscTimepoint finish);
    std::chrono::nanoseconds toNanos(TscDuration duration);

    static TscImpl singleton();
    double cpuClockRate();
    std::uint64_t rdtscCyclesCost();
    std::uint64_t realClockCyclesCost();
    void calibrate();
    void calibrateRdtscCost();
    void calibrateRealClockCost();
    void calibrateCpuClockRate();

   private:
    std::uint64_t _rdtscCyclesCost;
    std::uint64_t _realClockCyclesCost;
    double _cpuClockRate;
};

template <class Traits>
TscImpl<Traits>::TscImpl() {
    calibrate();
}

template <class Traits>
TscImpl<Traits> TscImpl<Traits>::singleton() {
    static TscImpl instance = TscImpl();
    return instance;
}

template <class Traits>
double TscImpl<Traits>::cpuClockRate() {
    return _cpuClockRate;
}

template <class Traits>
uint64_t TscImpl<Traits>::rdtscCyclesCost() {
    return _rdtscCyclesCost;
}

template <class Traits>
uint64_t TscImpl<Traits>::realClockCyclesCost() {
    return _realClockCyclesCost;
}

template <class Traits>
typename TscImpl<Traits>::TscTimepoint TscImpl<Traits>::now() {
    return __rdtsc();
}

template <class Traits>
typename TscImpl<Traits>::TscDuration TscImpl<Traits>::duration(TscTimepoint start, TscTimepoint finish) {
    return finish - start - _rdtscCyclesCost;
}

template <class Traits>
typename std::chrono::nanoseconds TscImpl<Traits>::toNanos(TscDuration duration) {
    return std::chrono::nanoseconds(static_cast<std::uint64_t>(duration / cpuClockRate()));
}

template <class Traits>
void TscImpl<Traits>::calibrate() {
    calibrateRdtscCost();
    calibrateRealClockCost();
    calibrateCpuClockRate();
}

template <class Traits>
void TscImpl<Traits>::calibrateRdtscCost() {
    std::vector<std::uint64_t> samples;
    samples.reserve(Traits::kCostMeasurements);
    auto lastRdtsc = now();

    while (samples.size() < Traits::kCostMeasurements) {
        std::int64_t duration = now() - lastRdtsc;
        samples.emplace_back(static_cast<std::uint64_t>(duration));
        lastRdtsc = now();
    }

    std::sort(samples.begin(), samples.end());
    _rdtscCyclesCost = samples[Traits::kCostMeasurements / 2];
}

template <class Traits>
void TscImpl<Traits>::calibrateRealClockCost() {
    std::vector<std::uint64_t> samples;
    samples.reserve(Traits::kCostMeasurements);

    while (samples.size() < Traits::kCostMeasurements) {
        const auto startTimepoint = std::chrono::system_clock::now();
        const auto startTsc = now();
        const auto finishTimepoint = std::chrono::system_clock::now();
        const auto finishTsc = now();

        if (finishTimepoint <= startTimepoint) {
            continue;
        }

        const TscDuration duration = finishTsc - startTsc - _rdtscCyclesCost * 2;
        samples.emplace_back(static_cast<std::uint64_t>(duration));
    }

    std::sort(samples.begin(), samples.end());
    _realClockCyclesCost = samples[Traits::kCostMeasurements / 2];
}

template <class Traits>
void TscImpl<Traits>::calibrateCpuClockRate() {
    std::vector<double> samples;
    samples.reserve(Traits::kRateMeasurements);

    while (samples.size() < Traits::kRateMeasurements) {
        const auto chronoStart = std::chrono::system_clock::now();
        const auto tscStart = now();

        while (true) {
            const auto chronoLast = std::chrono::system_clock::now();

            if (chronoLast <= chronoStart) {
                break;
            }

            const auto chronoDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(chronoLast - chronoStart);

            if (chronoDuration >= Traits::kRateMeasureDuration) {
                const auto tscFinish = now();
                samples.emplace_back(static_cast<double>(duration(tscStart, tscFinish) - _realClockCyclesCost) /
                                     chronoDuration.count());
                break;
            }
        }
    }

    std::sort(samples.begin(), samples.end());
    _cpuClockRate = samples[Traits::kRateMeasurements / 2];
}

}  // namespace tsc