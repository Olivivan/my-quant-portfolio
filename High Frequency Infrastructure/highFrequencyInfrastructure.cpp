#include <iostream>
#include <cmath>
#include <chrono>
#include <string>
#include <algorithm>
#include <array>

// ============================================================================
// 1. HIGH-FREQUENCY INFRASTRUCTURE: ZERO-ALLOCATION LIMIT ORDER BOOK
// ============================================================================
struct Order {
    uint64_t order_id;
    double price;
    uint32_t quantity;
};

// Fixed-size memory layout to completely avoid the OS heap allocator during trading hours
class LowLatencyOrderBook {
private:
    std::array<Order, 5> top_bids; // Pre-allocated storage for top 5 price levels
    size_t bid_count = 0;

public:
    // No allocations, no push_back. Memory-aligned insert.
    void insert_bid(uint64_t id, double price, uint32_t qty) {
        if (bid_count < top_bids.size()) {
            top_bids[bid_count++] = Order{id, price, qty};
            // Fast inline sort to maintain the highest bid at index 0
            std::sort(top_bids.begin(), top_bids.begin() + bid_count, 
                      [](const Order& a, const Order& b) { return a.price > b.price; });
        }
    }

    double get_best_bid() const {
        return (bid_count > 0) ? top_bids[0].price : 0.0;
    }
};

// ============================================================================
// 2. PRICING & RISK MATH: BLACK-SCHOLES EVALUATION ENGINE
// ============================================================================
class PricingEngine {
private:
    // Cumulative distribution function for standard normal distribution
    static inline double normal_cdf(double value) {
        return 0.5 * std::erfc(-value * std::sqrt(0.5));
    }

public:
    struct OptionGreeks {
        double call_price;
        double delta;
    };

    // Low-latency mathematical function using native CPU math optimizations
    static inline OptionGreeks calculate_call_option(double S, double K, double T, double r, double sigma) {
        // Handle edge case where market might cross or zero out
        if (S <= 0.0 || T <= 0.0) return OptionGreeks{0.0, 0.0};

        double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
        double d2 = d1 - sigma * std::sqrt(T);

        double call_price = S * normal_cdf(d1) - K * std::exp(-r * T) * normal_cdf(d2);
        double delta = normal_cdf(d1); // Delta represents sensitivity to stock movements

        return OptionGreeks{call_price, delta};
    }
};

// ============================================================================
// MAIN PIPELINE PIPELINE: COMBINING HFT AND QUANT PRICING
// ============================================================================
int main() {
    std::cout << "Initializing Low-Latency Multi-Component Engine...\n\n";

    // 1. Instantiating memory components
    LowLatencyOrderBook order_book;
    
    // Option Parameters: Strike (K)=100.0, Time to Expiry (T)=0.25 (3 months), Risk-Free Rate (r)=5%, Volatility (sigma)=20%
    const double STRIKE = 100.0;
    const double TIME_TO_MATURITY = 0.25;
    const double RISK_FREE_RATE = 0.05;
    const double VOLATILITY = 0.20;

    // 2. Simulate raw HFT network ticks arriving sequentially
    std::cout << "--- Ingesting Market Events ---\n";
    std::array<double, 3> incoming_ticks = {98.50, 101.20, 105.00};
    uint64_t fake_id = 5001;

    for (double market_price : incoming_ticks) {
        // Start microsecond timer
        auto start_time = std::chrono::high_resolution_clock::now();

        // STEP A: HFT Infrastructure updates the Order Book
        order_book.insert_bid(fake_id++, market_price, 1000);
        double current_underlying_stock = order_book.get_best_bid();

        // STEP B: Quantitative Analytics kicks off instantly using the book price
        PricingEngine::OptionGreeks greeks = PricingEngine::calculate_call_option(
            current_underlying_stock, STRIKE, TIME_TO_MATURITY, RISK_FREE_RATE, VOLATILITY
        );

        // Stop timer
        auto end_time = std::chrono::high_resolution_clock::now();
        auto processing_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

        // STEP C: Execution Decision Routing Path
        std::cout << "Stock Bid: $" << current_underlying_stock 
                  << " | Theoretical Call Option Price: $" << greeks.call_price 
                  << " | Delta: " << greeks.delta
                  << " | Latency: " << processing_nanoseconds << " ns\n";
    }

    std::cout << "\nExecution complete. Notice processing times are measured in nanoseconds due to zero heap allocations.\n";
    return 0;
}
