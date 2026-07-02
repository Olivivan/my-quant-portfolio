#include "RiskAnalytics.hpp"

namespace backtest {

namespace {

double safeMean(const std::vector<double>& values) noexcept {
    if (values.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (double value : values) {
        sum += value;
    }
    return sum / static_cast<double>(values.size());
}

double safeStdDev(const std::vector<double>& values, double mean) noexcept {
    if (values.size() < 2) {
        return 0.0;
    }
    double sumSq = 0.0;
    for (double value : values) {
        const double diff = value - mean;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / static_cast<double>(values.size() - 1));
}

} // namespace

RiskMetrics computeRiskMetrics(const std::vector<double>& equityCurve,
                               double periodsPerYear) noexcept {
    RiskMetrics metrics;
    if (equityCurve.size() < 2u) {
        return metrics;
    }

    const double startValue = equityCurve.front();
    const double endValue = equityCurve.back();
    if (startValue <= 0.0) {
        return metrics;
    }

    metrics.totalReturn = (endValue / startValue) - 1.0;

    const std::size_t periods = equityCurve.size() - 1;
    const double years = static_cast<double>(periods) / periodsPerYear;
    if (years > 0.0) {
        metrics.cagr = std::pow((endValue / startValue), 1.0 / years) - 1.0;
    }

    std::vector<double> returns;
    returns.reserve(periods);
    for (std::size_t i = 1; i < equityCurve.size(); ++i) {
        const double prev = equityCurve[i - 1];
        if (prev <= 0.0) {
            returns.push_back(0.0);
            continue;
        }
        returns.push_back((equityCurve[i] / prev) - 1.0);
    }

    const double meanReturn = safeMean(returns);
    const double returnStdDev = safeStdDev(returns, meanReturn);
    if (returnStdDev > 0.0) {
        metrics.sharpeRatio = meanReturn / returnStdDev * std::sqrt(periodsPerYear);
    }

    std::vector<double> downsideReturns;
    downsideReturns.reserve(returns.size());
    for (double dailyReturn : returns) {
        if (dailyReturn < 0.0) {
            downsideReturns.push_back(dailyReturn);
        }
    }
    const double downsideMean = safeMean(downsideReturns);
    const double downsideStdDev = safeStdDev(downsideReturns, downsideMean);
    if (downsideStdDev > 0.0) {
        metrics.sortinoRatio = meanReturn / downsideStdDev * std::sqrt(periodsPerYear);
    }

    double peak = equityCurve.front();
    double trough = equityCurve.front();
    std::size_t currentDuration = 0u;
    std::size_t longestDuration = 0u;
    double maxDrawdown = 0.0;

    for (double value : equityCurve) {
        if (value > peak) {
            peak = value;
            trough = value;
            currentDuration = 0u;
            continue;
        }

        if (value < trough) {
            trough = value;
        }

        const double drawdown = (peak - value) / peak;
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }

        ++currentDuration;
        if (currentDuration > longestDuration) {
            longestDuration = currentDuration;
        }
    }

    metrics.maxDrawdown = maxDrawdown;
    metrics.maxDrawdownDuration = longestDuration;
    return metrics;
}

} // namespace backtest
