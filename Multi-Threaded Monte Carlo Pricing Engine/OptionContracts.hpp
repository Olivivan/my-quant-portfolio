#ifndef OPTION_CONTRACTS_HPP
#define OPTION_CONTRACTS_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>

class ExoticPayoff {
public:
    virtual ~ExoticPayoff() noexcept = default;
    virtual void reset() noexcept = 0;
    virtual void observe(double underlying) noexcept = 0;
    virtual double payoff() const noexcept = 0;
    virtual std::unique_ptr<ExoticPayoff> clone() const noexcept = 0;
};

class AsianArithmeticMeanStrikeCall final : public ExoticPayoff {
public:
    explicit AsianArithmeticMeanStrikeCall(double strike) noexcept;

    void reset() noexcept override;
    void observe(double underlying) noexcept override;
    double payoff() const noexcept override;
    std::unique_ptr<ExoticPayoff> clone() const noexcept override;

private:
    double strike_;
    double sum_;
    std::size_t samples_;
    double lastPrice_;
};

class DownAndOutBarrierCall final : public ExoticPayoff {
public:
    DownAndOutBarrierCall(double strike, double barrier) noexcept;

    void reset() noexcept override;
    void observe(double underlying) noexcept override;
    double payoff() const noexcept override;
    std::unique_ptr<ExoticPayoff> clone() const noexcept override;

private:
    double strike_;
    double barrier_;
    bool knockedOut_;
    double lastPrice_;
};

inline AsianArithmeticMeanStrikeCall::AsianArithmeticMeanStrikeCall(double strike) noexcept
    : strike_(strike), sum_(0.0), samples_(0), lastPrice_(0.0) {}

inline void AsianArithmeticMeanStrikeCall::reset() noexcept {
    sum_ = 0.0;
    samples_ = 0;
    lastPrice_ = 0.0;
}

inline void AsianArithmeticMeanStrikeCall::observe(double underlying) noexcept {
    sum_ += underlying;
    samples_ += 1;
    lastPrice_ = underlying;
}

inline double AsianArithmeticMeanStrikeCall::payoff() const noexcept {
    if (samples_ == 0) {
        return 0.0;
    }

    const double arithmeticMean = sum_ / static_cast<double>(samples_);
    return std::max(lastPrice_ - arithmeticMean, 0.0);
}

inline std::unique_ptr<ExoticPayoff> AsianArithmeticMeanStrikeCall::clone() const noexcept {
    return std::make_unique<AsianArithmeticMeanStrikeCall>(strike_);
}

inline DownAndOutBarrierCall::DownAndOutBarrierCall(double strike, double barrier) noexcept
    : strike_(strike), barrier_(barrier), knockedOut_(false), lastPrice_(0.0) {}

inline void DownAndOutBarrierCall::reset() noexcept {
    knockedOut_ = false;
    lastPrice_ = 0.0;
}

inline void DownAndOutBarrierCall::observe(double underlying) noexcept {
    if (underlying <= barrier_) {
        knockedOut_ = true;
    }
    lastPrice_ = underlying;
}

inline double DownAndOutBarrierCall::payoff() const noexcept {
    if (knockedOut_) {
        return 0.0;
    }

    return std::max(lastPrice_ - strike_, 0.0);
}

inline std::unique_ptr<ExoticPayoff> DownAndOutBarrierCall::clone() const noexcept {
    return std::make_unique<DownAndOutBarrierCall>(strike_, barrier_);
}

#endif // OPTION_CONTRACTS_HPP
