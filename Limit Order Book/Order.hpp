#pragma once

#include <cstdint>

struct Order {
    using IdType = std::uint64_t;
    using PriceType = std::uint32_t;

    IdType id{0};
    bool is_buy{false};
    PriceType price{0};
    PriceType qty{0};
    Order* next{nullptr};
    Order* prev{nullptr};
};
