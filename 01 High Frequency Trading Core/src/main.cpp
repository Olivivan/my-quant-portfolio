#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "../lib/hft_core_lib/hft_core.hpp"
#include "../lib/hft_render_lib/hft_render.hpp"

namespace {
    enum class EventType {
        Limit,
        Cancel,
        Replace
    };

    struct SimulationEvent {
        EventType type;
        HFT::Order order;
        std::uint64_t targetOrderId;
    };

    std::string OptionalPrice(const std::optional<double>& price) {
        if (!price.has_value()) {
            return "-";
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << price.value();
        return out.str();
    }

    std::vector<SimulationEvent> BuildDeterministicScenario() {
        return {
            {EventType::Limit, {1, 100.00, 12, true, 1000}, 0},   // Resting bid
            {EventType::Limit, {2, 100.50, 10, true, 1001}, 0},   // Better bid
            {EventType::Limit, {3, 101.00, 8, false, 1002}, 0},   // Resting ask
            {EventType::Limit, {4, 100.50, 6, false, 1003}, 0},   // Crosses best bid
            {EventType::Replace, {0, 100.25, 9, true, 1004}, 1},  // Move id=1 upward
            {EventType::Cancel, {0, 0.0, 0, false, 0}, 3},        // Cancel ask id=3
            {EventType::Limit, {5, 100.20, 4, false, 1005}, 0},   // New ask
            {EventType::Limit, {6, 100.25, 11, true, 1006}, 0},   // Crosses ask, partial rest
            {EventType::Limit, {7, 100.20, 5, false, 1007}, 0},   // Sweeps residual bid
        };
    }

    int RunSimulation() {
        HFT::HFTCore engine;
        const std::vector<SimulationEvent> events = BuildDeterministicScenario();
        std::size_t accepted = 0;
        std::size_t rejected = 0;

        HFTRender::LogMetric("starting deterministic simulation");

        for (std::size_t i = 0; i < events.size(); ++i) {
            const SimulationEvent& ev = events[i];
            bool ok = false;
            if (ev.type == EventType::Limit) {
                ok = engine.LimitOrder(ev.order);
            } else if (ev.type == EventType::Cancel) {
                ok = engine.CancelOrder(ev.targetOrderId);
            } else {
                ok = engine.ReplaceOrder(
                    ev.targetOrderId,
                    ev.order.price,
                    ev.order.quantity,
                    ev.order.timestamp
                );
            }

            if (ok) {
                ++accepted;
            } else {
                ++rejected;
            }

            std::cout << "event " << (i + 1) << " -> " << (ok ? "ok" : "rejected") << '\n';
            HFTRender::RenderMarketData(engine.GetBestBidPrice(), engine.GetBestAskPrice());
        }

        std::cout << "\nsummary\n";
        std::cout << "accepted_events=" << accepted << '\n';
        std::cout << "rejected_events=" << rejected << '\n';
        std::cout << "best_bid=" << OptionalPrice(engine.GetBestBidPrice()) << '\n';
        std::cout << "best_ask=" << OptionalPrice(engine.GetBestAskPrice()) << '\n';
        std::cout << "buy_levels=" << engine.GetBuyBook().size() << '\n';
        std::cout << "sell_levels=" << engine.GetSellBook().size() << '\n';

        HFTRender::LogMetric("simulation completed");
        return 0;
    }

    int RunBenchmark() {
        HFT::HFTCore engine;
        constexpr std::uint64_t kOrderCount = 200000;
        auto start = std::chrono::steady_clock::now();

        for (std::uint64_t i = 1; i <= kOrderCount; ++i) {
            const bool isBuy = (i % 2 == 0);
            const double basePrice = isBuy ? 100.10 : 100.20;
            const double tickOffset = static_cast<double>(i % 5) * 0.01;
            const HFT::Order order{
                i,
                isBuy ? (basePrice - tickOffset) : (basePrice + tickOffset),
                static_cast<std::uint32_t>((i % 10) + 1),
                isBuy,
                static_cast<long>(100000 + i)
            };
            engine.LimitOrder(order);

            if ((i % 50000) == 0) {
                std::cout << "benchmark_progress=" << i << "\n";
            }
        }

        auto end = std::chrono::steady_clock::now();
        const double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        const double throughput = (static_cast<double>(kOrderCount) * 1000.0) / elapsedMs;

        std::cout << "benchmark_orders=" << kOrderCount << '\n';
        std::cout << "benchmark_elapsed_ms=" << std::fixed << std::setprecision(3) << elapsedMs << '\n';
        std::cout << "benchmark_orders_per_sec=" << std::fixed << std::setprecision(0) << throughput << '\n';
        std::cout << "benchmark_best_bid=" << OptionalPrice(engine.GetBestBidPrice()) << '\n';
        std::cout << "benchmark_best_ask=" << OptionalPrice(engine.GetBestAskPrice()) << '\n';
        return 0;
    }
}

int main(int argc, char** argv) {
    if (argc > 1) {
        const std::string mode = argv[1];
        if (mode == "--benchmark") {
            return RunBenchmark();
        }
        if (mode != "--simulate") {
            std::cerr << "usage: hft_main [--simulate|--benchmark]\n";
            return 2;
        }
    }

    return RunSimulation();
}