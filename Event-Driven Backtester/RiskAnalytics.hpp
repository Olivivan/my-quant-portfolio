#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

namespace backtest {

struct RiskMetrics {
    double totalReturn{};
    double cagr{};
    double sharpeRatio{};
    double sortinoRatio{};
    double maxDrawdown{};
    std::size_t maxDrawdownDuration{};
};

RiskMetrics computeRiskMetrics(const std::vector<double>& equityCurve,
                               double periodsPerYear = 252.0) noexcept;

} // namespace backtest
