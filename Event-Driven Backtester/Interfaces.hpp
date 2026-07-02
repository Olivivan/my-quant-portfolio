#pragma once

#include "Events.hpp"

namespace backtest {

class IDataHandler {
public:
    using EventQueue = backtest::EventQueue;
    virtual ~IDataHandler() = default;
    virtual bool hasData() const noexcept = 0;
    virtual void streamNext(EventQueue& eventQueue) noexcept = 0;
};

class IStrategy {
public:
    using EventQueue = backtest::EventQueue;
    virtual ~IStrategy() = default;
    virtual void onMarketEvent(const MarketEvent& marketEvent, EventQueue& eventQueue) noexcept = 0;
};

class IPortfolio {
public:
    using EventQueue = backtest::EventQueue;
    virtual ~IPortfolio() = default;
    virtual void onMarketEvent(const MarketEvent& marketEvent, EventQueue& eventQueue) noexcept = 0;
    virtual void onSignal(const SignalEvent& signalEvent, EventQueue& eventQueue) noexcept = 0;
    virtual void onFill(const FillEvent& fillEvent, EventQueue& eventQueue) noexcept = 0;
    virtual double currentEquity() const noexcept = 0;
};

class IExecutionHandler {
public:
    using EventQueue = backtest::EventQueue;
    virtual ~IExecutionHandler() = default;
    virtual void executeOrder(const OrderEvent& orderEvent, EventQueue& eventQueue) noexcept = 0;
};

} // namespace backtest
