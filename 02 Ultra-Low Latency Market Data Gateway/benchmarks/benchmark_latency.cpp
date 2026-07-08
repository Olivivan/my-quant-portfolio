#include "ull/feeds/feed_handler.hpp"
#include "ull/gateway/market_data_gateway.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::size_t kExpectedSamples = 20000;
constexpr double kFlatRatioTarget = 2.0;
constexpr std::size_t kRouteBatchOps = 64;
constexpr std::size_t kFeedBatchOps = 8;

void publish_latency_counters(benchmark::State& state, const std::vector<std::uint64_t>& samples_ns, double flat_ratio_target) {
    if (samples_ns.empty()) {
        state.SkipWithError("No latency samples collected");
        return;
    }

    std::vector<std::uint64_t> ordered(samples_ns);
    std::sort(ordered.begin(), ordered.end());

    const auto idx50 = static_cast<std::size_t>((ordered.size() - 1) * 0.50);
    const auto idx99 = static_cast<std::size_t>((ordered.size() - 1) * 0.99);

    const double p50 = static_cast<double>(ordered[idx50]);
    const double p99 = static_cast<double>(ordered[idx99]);
    const double flat_ratio = (p50 > 0.0) ? (p99 / p50) : 0.0;

    state.counters["p50_ns"] = benchmark::Counter(p50);
    state.counters["p99_ns"] = benchmark::Counter(p99);
    state.counters["flat_ratio"] = benchmark::Counter(flat_ratio);
    state.counters["flat_ok"] = benchmark::Counter(flat_ratio <= flat_ratio_target ? 1.0 : 0.0);
}

void BM_RouteMessageTypeLatency(benchmark::State& state) {
    ull::gateway::MarketDataGateway gateway;
    std::vector<std::uint64_t> samples;
    samples.reserve(kExpectedSamples);

    constexpr std::string_view type = "QUOTE";

    for (auto _ : state) {
        const auto t0 = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < kRouteBatchOps; ++i) {
            benchmark::DoNotOptimize(gateway.route_message_type(type));
        }
        const auto t1 = std::chrono::steady_clock::now();

        const auto raw_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        const auto batch_ns = (raw_ns <= 0) ? 1LL : raw_ns;
        const auto ns = (batch_ns / static_cast<long long>(kRouteBatchOps)) <= 0 ? 1LL : (batch_ns / static_cast<long long>(kRouteBatchOps));
        samples.push_back(static_cast<std::uint64_t>(ns));
        state.SetIterationTime(static_cast<double>(batch_ns) / 1'000'000'000.0);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * kRouteBatchOps));
    publish_latency_counters(state, samples, kFlatRatioTarget);
}

void BM_FeedHandlerPacketLatency(benchmark::State& state) {
    ull::feeds::FeedHandler handler;
    std::vector<std::uint64_t> samples;
    samples.reserve(kExpectedSamples);

    const std::string payload = std::string{"sym=NVDA"} + '\x01' + "px=1421.10" + '\x01' + "qty=50";

    for (auto _ : state) {
        const auto t0 = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < kFeedBatchOps; ++i) {
            benchmark::DoNotOptimize(handler.on_packet(payload));
        }
        const auto t1 = std::chrono::steady_clock::now();

        const auto raw_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        const auto batch_ns = (raw_ns <= 0) ? 1LL : raw_ns;
        const auto ns = (batch_ns / static_cast<long long>(kFeedBatchOps)) <= 0 ? 1LL : (batch_ns / static_cast<long long>(kFeedBatchOps));
        samples.push_back(static_cast<std::uint64_t>(ns));
        state.SetIterationTime(static_cast<double>(batch_ns) / 1'000'000'000.0);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * kFeedBatchOps));
    publish_latency_counters(state, samples, kFlatRatioTarget);
}

} // namespace

BENCHMARK(BM_RouteMessageTypeLatency)
    ->Iterations(kExpectedSamples)
    ->UseManualTime();

BENCHMARK(BM_FeedHandlerPacketLatency)
    ->Iterations(kExpectedSamples)
    ->UseManualTime();
