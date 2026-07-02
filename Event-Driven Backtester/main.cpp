#include "BacktestEngine.hpp"
#include "Events.hpp"
#include "Interfaces.hpp"
#include "RiskAnalytics.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace backtest;

class CsvBarDataHandler final : public IDataHandler {
public:
    explicit CsvBarDataHandler(std::string_view path, std::string_view symbol) noexcept
        : filePath_(path), symbol_(symbol), inputStream_(), currentTimestamp_(),
          nextOpen_(0.0), nextHigh_(0.0), nextLow_(0.0), nextClose_(0.0), nextVolume_(0.0),
          hasNext_(false) {
        inputStream_.open(std::string(filePath_));
        if (!inputStream_) {
            return;
        }
        hasNext_ = readNextBar();
    }

    bool hasData() const noexcept override {
        return hasNext_;
    }

    void streamNext(EventQueue& eventQueue) noexcept override {
        if (!hasNext_) {
            return;
        }

        const auto event = std::make_shared<MarketEvent>(currentTimestamp_, symbol_, nextOpen_, nextHigh_, nextLow_, nextClose_, nextVolume_);
        eventQueue.push(event);
        hasNext_ = readNextBar();
    }

private:
    bool readNextBar() noexcept {
        if (!inputStream_ || inputStream_.eof()) {
            return false;
        }

        std::string line;
        if (!std::getline(inputStream_, line)) {
            return false;
        }

        if (line.empty() || line.rfind("Date", 0) == 0) {
            return static_cast<bool>(std::getline(inputStream_, line));
        }

        std::istringstream row(line);
        std::string dateToken;
        std::string openToken;
        std::string highToken;
        std::string lowToken;
        std::string closeToken;
        std::string volumeToken;

        if (!std::getline(row, dateToken, ',') || !std::getline(row, openToken, ',') ||
            !std::getline(row, highToken, ',') || !std::getline(row, lowToken, ',') ||
            !std::getline(row, closeToken, ',') || !std::getline(row, volumeToken, ',')) {
            return false;
        }

        std::tm tm = {};
        std::istringstream dateStream(dateToken);
        dateStream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        currentTimestamp_ = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        nextOpen_ = std::stod(openToken);
        nextHigh_ = std::stod(highToken);
        nextLow_ = std::stod(lowToken);
        nextClose_ = std::stod(closeToken);
        nextVolume_ = std::stod(volumeToken);
        return true;
    }

    std::string filePath_;
    std::string symbol_;
    mutable std::ifstream inputStream_;
    Event::TimePoint currentTimestamp_;
    double nextOpen_;
    double nextHigh_;
    double nextLow_;
    double nextClose_;
    double nextVolume_;
    bool hasNext_;
};

class MovingAverageStrategy final : public IStrategy {
public:
    explicit MovingAverageStrategy(std::size_t shortWindow, std::size_t longWindow) noexcept
        : shortWindow_(shortWindow), longWindow_(longWindow), closePrices_(), positionOpen_(false) {}

    void onMarketEvent(const MarketEvent& marketEvent, EventQueue& eventQueue) noexcept override {
        closePrices_.push_back(marketEvent.close);
        if (closePrices_.size() < longWindow_) {
            return;
        }

        const double shortMa = computeMa(shortWindow_);
        const double longMa = computeMa(longWindow_);

        if (!positionOpen_ && shortMa > longMa) {
            const auto signal = std::make_shared<SignalEvent>(marketEvent.timestamp, marketEvent.symbol, Direction::Long, 1.0);
            eventQueue.push(signal);
            positionOpen_ = true;
        } else if (positionOpen_ && shortMa < longMa) {
            const auto signal = std::make_shared<SignalEvent>(marketEvent.timestamp, marketEvent.symbol, Direction::Exit, 1.0);
            eventQueue.push(signal);
            positionOpen_ = false;
        }
    }

private:
    double computeMa(std::size_t window) const noexcept {
        const std::size_t size = closePrices_.size();
        if (window == 0 || window > size) {
            return 0.0;
        }
        const auto start = closePrices_.end() - static_cast<std::ptrdiff_t>(window);
        double sum = 0.0;
        for (auto it = start; it != closePrices_.end(); ++it) {
            sum += *it;
        }
        return sum / static_cast<double>(window);
    }

    std::size_t shortWindow_;
    std::size_t longWindow_;
    std::vector<double> closePrices_;
    bool positionOpen_;
};

class PortfolioManager final : public IPortfolio {
public:
    explicit PortfolioManager(double initialCapital, std::size_t fixedSize) noexcept
        : cash_(initialCapital), positionSize_(0), currentPrice_(0.0), equity_(initialCapital), fixedSize_(fixedSize) {}

    void onMarketEvent(const MarketEvent& marketEvent, EventQueue&) noexcept override {
        currentPrice_ = marketEvent.close;
        equity_ = cash_ + static_cast<double>(positionSize_) * currentPrice_;
    }

    void onSignal(const SignalEvent& signalEvent, EventQueue& eventQueue) noexcept override {
        if (signalEvent.direction == Direction::Long) {
            const auto order = std::make_shared<OrderEvent>(signalEvent.timestamp, signalEvent.symbol, Direction::Long, OrderType::Market, fixedSize_);
            eventQueue.push(order);
        } else if (signalEvent.direction == Direction::Exit && positionSize_ > 0) {
            const auto order = std::make_shared<OrderEvent>(signalEvent.timestamp, signalEvent.symbol, Direction::Exit, OrderType::Market, positionSize_);
            eventQueue.push(order);
        }
    }

    void onFill(const FillEvent& fillEvent, EventQueue&) noexcept override {
        const double fillCost = static_cast<double>(fillEvent.quantity) * fillEvent.fillPrice + fillEvent.commission;
        if (fillEvent.direction == Direction::Long) {
            cash_ -= fillCost;
            positionSize_ += static_cast<std::size_t>(fillEvent.quantity);
        } else if (fillEvent.direction == Direction::Exit) {
            cash_ += fillCost;
            positionSize_ = 0;
        }
        equity_ = cash_ + static_cast<double>(positionSize_) * currentPrice_;
    }

    double currentEquity() const noexcept override {
        return equity_;
    }

private:
    double cash_;
    std::size_t positionSize_;
    double currentPrice_;
    double equity_;
    std::size_t fixedSize_;
};

class SimulationExecutionHandler final : public IExecutionHandler {
public:
    SimulationExecutionHandler(double commissionPerOrder, double slippagePct) noexcept
        : commission_(commissionPerOrder), slippagePct_(slippagePct) {}

    void executeOrder(const OrderEvent& orderEvent, EventQueue& eventQueue) noexcept override {
        const double fillPrice = orderEvent.orderType == OrderType::Market ? applySlippage(orderEvent) : orderEvent.limitPrice;
        const auto fillEvent = std::make_shared<FillEvent>(orderEvent.timestamp, orderEvent.symbol, orderEvent.direction, orderEvent.quantity, fillPrice, commission_);
        eventQueue.push(fillEvent);
    }

private:
    double applySlippage(const OrderEvent& orderEvent) const noexcept {
        const double midPrice = orderEvent.limitPrice > 0.0 ? orderEvent.limitPrice : 0.0;
        const double slippage = midPrice * slippagePct_;
        if (orderEvent.direction == Direction::Long) {
            return midPrice + slippage;
        }
        return midPrice - slippage;
    }

    double commission_;
    double slippagePct_;
};

int main() {
    constexpr double initialCapital = 100000.0;
    constexpr std::size_t positionSize = 1000u;

    CsvBarDataHandler dataHandler("data.csv", "TEST");
    MovingAverageStrategy strategy(20u, 50u);
    PortfolioManager portfolio(initialCapital, positionSize);
    SimulationExecutionHandler executionHandler(1.0, 0.0005);

    BacktestEngine engine(dataHandler, strategy, portfolio, executionHandler);
    engine.run();

    const auto& equityCurve = engine.equityCurve();
    const auto metrics = computeRiskMetrics(equityCurve, 252.0);

    std::cout << "Final Equity: " << std::fixed << std::setprecision(2) << (equityCurve.empty() ? initialCapital : equityCurve.back()) << "\n";
    std::cout << "Total Return: " << metrics.totalReturn * 100.0 << "%\n";
    std::cout << "CAGR: " << metrics.cagr * 100.0 << "%\n";
    std::cout << "Sharpe Ratio: " << metrics.sharpeRatio << "\n";
    std::cout << "Sortino Ratio: " << metrics.sortinoRatio << "\n";
    std::cout << "Max Drawdown: " << metrics.maxDrawdown * 100.0 << "%\n";
    std::cout << "Max Drawdown Duration: " << metrics.maxDrawdownDuration << " bars\n";

    return 0;
}
