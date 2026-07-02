#pragma once

#include "Events.hpp"
#include "Interfaces.hpp"
#include <vector>

namespace backtest {

class BacktestEngine {
public:
    using EventQueue = backtest::EventQueue;

    BacktestEngine(IDataHandler& dataHandler,
                   IStrategy& strategy,
                   IPortfolio& portfolio,
                   IExecutionHandler& executionHandler) noexcept;

    void run() noexcept;
    [[nodiscard]] const std::vector<double>& equityCurve() const noexcept;

private:
    IDataHandler& dataHandler_;
    IStrategy& strategy_;
    IPortfolio& portfolio_;
    IExecutionHandler& executionHandler_;
    EventQueue eventQueue_;
    std::vector<double> equityCurve_;
};

} // namespace backtest
