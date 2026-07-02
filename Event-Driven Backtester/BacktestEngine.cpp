#include "BacktestEngine.hpp"
#include "Events.hpp"
#include "Interfaces.hpp"
#include <memory>

namespace backtest {

BacktestEngine::BacktestEngine(IDataHandler& dataHandler,
                               IStrategy& strategy,
                               IPortfolio& portfolio,
                               IExecutionHandler& executionHandler) noexcept
    : dataHandler_(dataHandler),
      strategy_(strategy),
      portfolio_(portfolio),
      executionHandler_(executionHandler),
      equityCurve_() {}

void BacktestEngine::run() noexcept {
    while (dataHandler_.hasData() || !eventQueue_.empty()) {
        if (eventQueue_.empty() && dataHandler_.hasData()) {
            dataHandler_.streamNext(eventQueue_);
        }

        if (eventQueue_.empty()) {
            break;
        }

        const auto event = eventQueue_.top();
        eventQueue_.pop();

        switch (event->type()) {
            case EventType::Market: {
                auto const& marketEvent = static_cast<const MarketEvent&>(*event);
                strategy_.onMarketEvent(marketEvent, eventQueue_);
                portfolio_.onMarketEvent(marketEvent, eventQueue_);
                break;
            }
            case EventType::Signal: {
                auto const& signalEvent = static_cast<const SignalEvent&>(*event);
                portfolio_.onSignal(signalEvent, eventQueue_);
                break;
            }
            case EventType::Order: {
                auto const& orderEvent = static_cast<const OrderEvent&>(*event);
                executionHandler_.executeOrder(orderEvent, eventQueue_);
                break;
            }
            case EventType::Fill: {
                auto const& fillEvent = static_cast<const FillEvent&>(*event);
                portfolio_.onFill(fillEvent, eventQueue_);
                equityCurve_.push_back(portfolio_.currentEquity());
                break;
            }
            default:
                break;
        }

        if (eventQueue_.empty() && dataHandler_.hasData()) {
            dataHandler_.streamNext(eventQueue_);
        }
    }
}

const std::vector<double>& BacktestEngine::equityCurve() const noexcept {
    return equityCurve_;
}

} // namespace backtest
