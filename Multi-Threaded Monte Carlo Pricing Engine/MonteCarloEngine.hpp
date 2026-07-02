#ifndef MONTE_CARLO_ENGINE_HPP
#define MONTE_CARLO_ENGINE_HPP

#include "OptionContracts.hpp"
#include "PathGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

struct MonteCarloResult {
    double price{0.0};
    double standardError{0.0};
    std::size_t simulatedPaths{0};
};

class MonteCarloEngine {
public:
    MonteCarloEngine(double initialSpot,
                     double rate,
                     double volatility,
                     double maturity,
                     std::size_t timeSteps,
                     std::uint64_t seed = 0) noexcept;

    MonteCarloResult simulate(const ExoticPayoff& payoffPrototype,
                              std::size_t pathCount,
                              std::size_t threadCount,
                              bool useAntithetic) const noexcept;

private:
    struct alignas(64) ThreadAccumulator {
        double sum{0.0};
        double sumSq{0.0};
    };

    void workerThread(std::size_t assignedPaths,
                      const ExoticPayoff& payoffPrototype,
                      bool useAntithetic,
                      ThreadAccumulator& accumulator,
                      std::uint64_t seed) const noexcept;

    double initialSpot_;
    double rate_;
    double volatility_;
    double maturity_;
    std::size_t timeSteps_;
    std::uint64_t seed_;
};

#endif // MONTE_CARLO_ENGINE_HPP
