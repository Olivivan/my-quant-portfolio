#include "AnalyticsEngine.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

using namespace tseries;

int main() {
    constexpr size_t kSize = 50'000'000;
    constexpr size_t kWindow = 50;

    Vector<double> prices(kSize);
    for (size_t i = 0; i < kSize; ++i) {
        prices[i] = 100.0 + std::sin(static_cast<double>(i) * 0.00013) * 2.5 + std::cos(static_cast<double>(i) * 0.00019) * 1.8;
    }

    const auto benchmark = [&](auto const& label, auto&& fn) {
        const auto start = std::chrono::high_resolution_clock::now();
        fn();
        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << label << " elapsed: " << elapsed.count() << " seconds\n";
    };

    std::cout << "Benchmarking TimeSeries Analytics with " << kSize << " points...\n";

    Vector<double> ema_result;
    benchmark("Library rolling_ema", [&] {
        ema_result = rolling_ema<double>(prices, kWindow);
    });

    std::vector<double> native_ema(kSize);
    benchmark("Native loop rolling_ema", [&] {
        const double alpha = 2.0 / (static_cast<double>(kWindow) + 1.0);
        native_ema[0] = prices[0];
        for (size_t i = 1; i < kSize; ++i) {
            native_ema[i] = alpha * (prices[i] - native_ema[i - 1]) + native_ema[i - 1];
        }
    });

    Vector<double> volatility_result;
    benchmark("Library rolling_volatility", [&] {
        volatility_result = rolling_volatility<double>(prices, kWindow);
    });

    std::vector<double> native_volatility(kSize, 0.0);
    benchmark("Native loop rolling_volatility", [&] {
        double mean = 0.0;
        double m2 = 0.0;
        size_t count = 0;
        for (size_t i = 0; i < kSize; ++i) {
            const double value = prices[i];
            if (count < kWindow) {
                ++count;
                const double delta = value - mean;
                mean += delta / static_cast<double>(count);
                const double delta2 = value - mean;
                m2 += delta * delta2;
            } else {
                const double old_value = prices[i - kWindow];
                const double delta_remove = old_value - mean;
                const double new_mean = mean - delta_remove / static_cast<double>(kWindow);
                const double m2_remove = m2 - delta_remove * (old_value - new_mean);
                mean = new_mean;
                m2 = m2_remove;
                const double delta_add = value - mean;
                mean += delta_add / static_cast<double>(kWindow);
                const double delta2_add = value - mean;
                m2 += delta_add * delta2_add;
            }
            if (i + 1 >= kWindow) {
                const double variance = m2 / static_cast<double>(kWindow - 1);
                native_volatility[i] = std::sqrt(variance);
            }
        }
    });

    Vector<double> X(kSize), Y(kSize);
    for (size_t i = 0; i < kSize; ++i) {
        X[i] = static_cast<double>(i);
        Y[i] = 0.25 * X[i] + 1.5 + std::sin(static_cast<double>(i) * 1e-6);
    }

    benchmark("Library linear_regression", [&] {
        auto result = linear_regression<double>(X, Y);
        std::cout << "  alpha=" << result.alpha << " beta=" << result.beta << " R^2=" << result.r_squared << "\n";
    });

    std::cout << "Benchmark complete." << std::endl;
    return 0;
}

// Compile with:
// g++ -std=c++20 -O3 -march=native main.cpp -o timeseries_bench
// clang++ -std=c++20 -O3 -march=native main.cpp -o timeseries_bench
