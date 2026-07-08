#pragma once

#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

/**
 * @brief Lightweight rendering and logging helpers for CLI output.
 */
namespace HFTRender {
    /**
     * @brief Render top-of-book values in a fixed textual format.
     * @param bestBid Best bid, if any.
     * @param bestAsk Best ask, if any.
     */
    inline void RenderMarketData(const std::optional<double>& bestBid, const std::optional<double>& bestAsk) {
        std::cout << "TOB | bid=";
        if (bestBid.has_value()) {
            std::cout << std::fixed << std::setprecision(2) << bestBid.value();
        } else {
            std::cout << "-";
        }

        std::cout << " ask=";
        if (bestAsk.has_value()) {
            std::cout << std::fixed << std::setprecision(2) << bestAsk.value();
        } else {
            std::cout << "-";
        }
        std::cout << '\n';
    }

    /**
     * @brief Emit a prefixed metric log line to stdout.
     * @param message Message body to print.
     */
    inline void LogMetric(const std::string& message) {
        std::cout << "[metric] " << message << '\n';
    }
}