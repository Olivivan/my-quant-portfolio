#ifndef PATH_GENERATOR_HPP
#define PATH_GENERATOR_HPP

#include "OptionContracts.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

class PathGenerator {
public:
    PathGenerator(double initialSpot,
                  double rate,
                  double volatility,
                  double maturity,
                  std::size_t timeSteps,
                  std::uint64_t seed = std::random_device{}()) noexcept;

    void setSeed(std::uint64_t seed) noexcept;
    void configure(double initialSpot,
                   double rate,
                   double volatility,
                   double maturity,
                   std::size_t timeSteps) noexcept;

    double simulatePath(ExoticPayoff& payoff) noexcept;
    std::pair<double, double> simulateAntitheticPair(ExoticPayoff& payoffA,
                                                     ExoticPayoff& payoffB) noexcept;

private:
    double initialSpot_;
    double rate_;
    double volatility_;
    double dt_;
    std::size_t timeSteps_;
    std::vector<double> primaryTrajectory_;
    std::vector<double> antitheticTrajectory_;
    std::mt19937_64 rng_;
    std::normal_distribution<double> distribution_;
    static constexpr double oneHalf = 0.5;
};

#endif // PATH_GENERATOR_HPP
