#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "OrderBook.hpp"

namespace {
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;
using std::cout;
using std::mt19937_64;
using std::uniform_int_distribution;
using std::vector;
}

int main() {
    MultiThreadedOrderBook book(200'000, 4, 256);

    mt19937_64 engine{42};
    uniform_int_distribution<int> side_dist(0, 1);
    uniform_int_distribution<int> price_dist(900, 1100);
    uniform_int_distribution<int> qty_dist(1, 50);

    vector<OrderCommand> commands;
    commands.reserve(100'000);

    for (int i = 0; i < 100'000; ++i) {
        OrderCommand command{};
        command.id = static_cast<OrderCommand::IdType>(i + 1);
        command.is_buy = side_dist(engine) == 1;
        command.price = static_cast<OrderCommand::PriceType>(price_dist(engine));
        command.qty = static_cast<OrderCommand::PriceType>(qty_dist(engine));
        commands.push_back(command);
    }

    const auto start = Clock::now();
    book.submit_batch(commands);
    const auto end = Clock::now();
    const auto elapsed_ms = std::chrono::duration_cast<Duration>(end - start).count();

    cout << "Active orders: " << book.active_order_count() << '\n';
    cout << "Executions: " << book.total_executions() << '\n';
    cout << "Elapsed ms: " << elapsed_ms << '\n';
    return 0;
}
