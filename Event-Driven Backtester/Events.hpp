#pragma once

#include <chrono>
#include <memory>
#include <string_view>
#include <vector>

namespace backtest {

enum class EventType {
    Market,
    Signal,
    Order,
    Fill
};

enum class Direction {
    Long,
    Short,
    Exit
};

enum class OrderType {
    Market,
    Limit
};

struct Event {
    using TimePoint = std::chrono::system_clock::time_point;

    Event(TimePoint timeStamp, int priority) noexcept
        : timestamp(timeStamp), priority(priority) {}

    virtual ~Event() = default;
    virtual EventType type() const noexcept = 0;
    virtual std::string_view name() const noexcept = 0;

    TimePoint timestamp;
    int priority;
};

struct MarketEvent final : Event {
    MarketEvent(TimePoint timeStamp, std::string_view symbol, double openPrice,
                double highPrice, double lowPrice, double closePrice,
                double volume) noexcept
        : Event(timeStamp, 0), symbol(symbol), open(openPrice), high(highPrice),
          low(lowPrice), close(closePrice), volume(volume) {}

    EventType type() const noexcept override { return EventType::Market; }
    std::string_view name() const noexcept override { return "MarketEvent"; }

    std::string_view symbol;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

struct SignalEvent final : Event {
    SignalEvent(TimePoint timeStamp, std::string_view symbol, Direction direction,
                double strength) noexcept
        : Event(timeStamp, 1), symbol(symbol), direction(direction), strength(strength) {}

    EventType type() const noexcept override { return EventType::Signal; }
    std::string_view name() const noexcept override { return "SignalEvent"; }

    std::string_view symbol;
    Direction direction;
    double strength;
};

struct OrderEvent final : Event {
    OrderEvent(TimePoint timeStamp, std::string_view symbol, Direction direction,
               OrderType orderType, std::size_t quantity, double limitPrice = 0.0) noexcept
        : Event(timeStamp, 2), symbol(symbol), direction(direction), orderType(orderType),
          quantity(quantity), limitPrice(limitPrice) {}

    EventType type() const noexcept override { return EventType::Order; }
    std::string_view name() const noexcept override { return "OrderEvent"; }

    std::string_view symbol;
    Direction direction;
    OrderType orderType;
    std::size_t quantity;
    double limitPrice;
};

struct FillEvent final : Event {
    FillEvent(TimePoint timeStamp, std::string_view symbol, Direction direction,
              std::size_t quantity, double fillPrice, double commission) noexcept
        : Event(timeStamp, 3), symbol(symbol), direction(direction), quantity(quantity),
          fillPrice(fillPrice), commission(commission) {}

    EventType type() const noexcept override { return EventType::Fill; }
    std::string_view name() const noexcept override { return "FillEvent"; }

    std::string_view symbol;
    Direction direction;
    std::size_t quantity;
    double fillPrice;
    double commission;
};

using EventPtr = std::shared_ptr<Event>;

struct EventComparator {
    bool operator()(EventPtr const& lhs, EventPtr const& rhs) const noexcept {
        if (lhs->timestamp != rhs->timestamp) {
            return lhs->timestamp > rhs->timestamp;
        }
        return lhs->priority > rhs->priority;
    }
};

using EventQueue = std::priority_queue<EventPtr, std::vector<EventPtr>, EventComparator>;

} // namespace backtest
