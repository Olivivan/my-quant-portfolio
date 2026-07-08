#pragma once

#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

namespace HFTRender {
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

    inline void LogMetric(const std::string& message) {
        std::cout << "[metric] " << message << '\n';
    }
}