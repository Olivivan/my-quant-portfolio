#include "PathGenerator.hpp"

#include <cmath>
#include <utility>

PathGenerator::PathGenerator(double initialSpot,
                             double rate,
                             double volatility,
                             double maturity,
                             std::size_t timeSteps,
                             std::uint64_t seed) noexcept
    : initialSpot_(initialSpot),
      rate_(rate),
      volatility_(volatility),
      dt_(timeSteps > 0 ? maturity / static_cast<double>(timeSteps) : 0.0),
      timeSteps_(timeSteps),
      primaryTrajectory_(timeSteps + 1),
      antitheticTrajectory_(timeSteps + 1),
      rng_(seed),
      distribution_(0.0, 1.0) {
    primaryTrajectory_[0] = initialSpot_;
    antitheticTrajectory_[0] = initialSpot_;
}

void PathGenerator::setSeed(std::uint64_t seed) noexcept {
    rng_.seed(seed);
}

void PathGenerator::configure(double initialSpot,
                              double rate,
                              double volatility,
                              double maturity,
                              std::size_t timeSteps) noexcept {
    initialSpot_ = initialSpot;
    rate_ = rate;
    volatility_ = volatility;
    timeSteps_ = timeSteps;
    dt_ = timeSteps > 0 ? maturity / static_cast<double>(timeSteps) : 0.0;
    primaryTrajectory_.assign(timeSteps + 1, initialSpot_);
    antitheticTrajectory_.assign(timeSteps + 1, initialSpot_);
}

double PathGenerator::simulatePath(ExoticPayoff& payoff) noexcept {
    payoff.reset();
    double currentSpot = initialSpot_;
    const double drift = (rate_ - oneHalf * volatility_ * volatility_) * dt_;
    const double diffusionScale = volatility_ * std::sqrt(dt_);

    for (std::size_t step = 1; step <= timeSteps_; ++step) {
        const double z = distribution_(rng_);
        currentSpot *= std::exp(drift + diffusionScale * z);
        primaryTrajectory_[step] = currentSpot;
        payoff.observe(currentSpot);
    }

    const double payoffValue = payoff.payoff();
    return payoffValue * std::exp(-rate_ * dt_ * static_cast<double>(timeSteps_));
}

std::pair<double, double> PathGenerator::simulateAntitheticPair(ExoticPayoff& payoffA,
                                                               ExoticPayoff& payoffB) noexcept {
    payoffA.reset();
    payoffB.reset();
    double spotA = initialSpot_;
    double spotB = initialSpot_;
    const double drift = (rate_ - oneHalf * volatility_ * volatility_) * dt_;
    const double diffusionScale = volatility_ * std::sqrt(dt_);

    for (std::size_t step = 1; step <= timeSteps_; ++step) {
        const double z = distribution_(rng_);
        const double exponentA = drift + diffusionScale * z;
        const double exponentB = drift - diffusionScale * z;
        spotA *= std::exp(exponentA);
        spotB *= std::exp(exponentB);
        primaryTrajectory_[step] = spotA;
        antitheticTrajectory_[step] = spotB;
        payoffA.observe(spotA);
        payoffB.observe(spotB);
    }

    const double payoffAValue = payoffA.payoff() * std::exp(-rate_ * dt_ * static_cast<double>(timeSteps_));
    const double payoffBValue = payoffB.payoff() * std::exp(-rate_ * dt_ * static_cast<double>(timeSteps_));
    return {payoffAValue, payoffBValue};
}
