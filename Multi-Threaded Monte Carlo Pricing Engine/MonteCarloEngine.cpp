#include "MonteCarloEngine.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

MonteCarloEngine::MonteCarloEngine(double initialSpot,
                                   double rate,
                                   double volatility,
                                   double maturity,
                                   std::size_t timeSteps,
                                   std::uint64_t seed) noexcept
    : initialSpot_(initialSpot),
      rate_(rate),
      volatility_(volatility),
      maturity_(maturity),
      timeSteps_(timeSteps),
      seed_(seed) {}

MonteCarloResult MonteCarloEngine::simulate(const ExoticPayoff& payoffPrototype,
                                             std::size_t pathCount,
                                             std::size_t threadCount,
                                             bool useAntithetic) const noexcept {
    if (pathCount == 0) {
        return {};
    }

    threadCount = std::max<std::size_t>(1u, threadCount);
    threadCount = std::min(threadCount, pathCount);

    std::vector<ThreadAccumulator> accumulators(threadCount);
    std::vector<std::jthread> workers;
    workers.reserve(threadCount);

    const std::size_t baseCount = pathCount / threadCount;
    const std::size_t remainder = pathCount % threadCount;

    for (std::size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        const std::size_t assignedPaths = baseCount + (threadIndex < remainder ? 1 : 0);
        const std::uint64_t threadSeed = seed_ + static_cast<std::uint64_t>(threadIndex + 1);
        workers.emplace_back(
            [this, &payoffPrototype, assignedPaths, useAntithetic, threadSeed, &accumulators, threadIndex]() noexcept {
                workerThread(assignedPaths, payoffPrototype, useAntithetic, accumulators[threadIndex], threadSeed);
            });
    }

    double totalSum = 0.0;
    double totalSumSq = 0.0;

    for (const auto& accumulator : accumulators) {
        totalSum += accumulator.sum;
        totalSumSq += accumulator.sumSq;
    }

    const double n = static_cast<double>(pathCount);
    const double mean = totalSum / n;
    const double variance = std::max(0.0, totalSumSq / n - mean * mean);
    const double standardError = std::sqrt(variance / n);

    return {mean, standardError, pathCount};
}

void MonteCarloEngine::workerThread(std::size_t assignedPaths,
                                    const ExoticPayoff& payoffPrototype,
                                    bool useAntithetic,
                                    ThreadAccumulator& accumulator,
                                    std::uint64_t seed) const noexcept {
    if (assignedPaths == 0) {
        return;
    }

    PathGenerator generator(initialSpot_, rate_, volatility_, maturity_, timeSteps_, seed);
    const std::unique_ptr<ExoticPayoff> payoffA = payoffPrototype.clone();
    const std::unique_ptr<ExoticPayoff> payoffB = payoffPrototype.clone();

    if (useAntithetic) {
        const std::size_t pairCount = assignedPaths / 2;
        const bool hasOdd = (assignedPaths % 2) != 0;

        for (std::size_t index = 0; index < pairCount; ++index) {
            const auto [firstPayoff, secondPayoff] = generator.simulateAntitheticPair(*payoffA, *payoffB);
            accumulator.sum += firstPayoff + secondPayoff;
            accumulator.sumSq += firstPayoff * firstPayoff + secondPayoff * secondPayoff;
        }

        if (hasOdd) {
            const double payoffValue = generator.simulatePath(*payoffA);
            accumulator.sum += payoffValue;
            accumulator.sumSq += payoffValue * payoffValue;
        }
    } else {
        for (std::size_t index = 0; index < assignedPaths; ++index) {
            const double payoffValue = generator.simulatePath(*payoffA);
            accumulator.sum += payoffValue;
            accumulator.sumSq += payoffValue * payoffValue;
        }
    }
}
