#include "MonteCarloEngine.hpp"
#include "OptionContracts.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>

int main() {
    const double initialSpot = 100.0;
    const double riskFreeRate = 0.03;
    const double volatility = 0.20;
    const double maturity = 1.0;
    const double strike = 100.0;
    const double barrierLevel = 90.0;
    const std::size_t timeSteps = 252;
    const std::size_t totalPaths = 10'000'000;
    const std::uint64_t seed = 0xC0FFEEu;

    const std::size_t hardwareThreads = std::max<std::size_t>(1u, std::thread::hardware_concurrency());
    std::vector<std::size_t> threadCounts = {1, 2, 4, 8, hardwareThreads};
    std::sort(threadCounts.begin(), threadCounts.end());
    threadCounts.erase(std::unique(threadCounts.begin(), threadCounts.end()), threadCounts.end());

    MonteCarloEngine engine(initialSpot, riskFreeRate, volatility, maturity, timeSteps, seed);

    std::cout << "Multi-Threaded Monte Carlo Pricing Benchmark\n";
    std::cout << "Paths: " << totalPaths << " | Time steps: " << timeSteps << " | Seed: " << seed << "\n\n";
    std::cout << std::left << std::setw(10) << "Workers" << std::setw(22) << "Contract"
              << std::setw(16) << "Price" << std::setw(18) << "Std. Error"
              << std::setw(14) << "Elapsed ms" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto workerCount : threadCounts) {
        AsianArithmeticMeanStrikeCall asianCall(strike);
        DownAndOutBarrierCall barrierCall(strike, barrierLevel);

        const auto startAsian = std::chrono::high_resolution_clock::now();
        const MonteCarloResult asianResult = engine.simulate(asianCall, totalPaths, workerCount, true);
        const auto elapsedAsian = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startAsian);

        const auto startBarrier = std::chrono::high_resolution_clock::now();
        const MonteCarloResult barrierResult = engine.simulate(barrierCall, totalPaths, workerCount, true);
        const auto elapsedBarrier = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startBarrier);

        std::cout << std::left << std::setw(10) << workerCount << std::setw(22) << "Asian Arithmetic Call"
                  << std::setw(16) << asianResult.price << std::setw(18) << asianResult.standardError
                  << std::setw(14) << elapsedAsian.count() << "\n";

        std::cout << std::left << std::setw(10) << workerCount << std::setw(22) << "Down-and-Out Barrier"
                  << std::setw(16) << barrierResult.price << std::setw(18) << barrierResult.standardError
                  << std::setw(14) << elapsedBarrier.count() << "\n";
    }

    std::cout << "\nAntithetic variance reduction is enabled for the benchmark runs.\n";
    return 0;
}
