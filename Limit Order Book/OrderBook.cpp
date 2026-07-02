#include "OrderBook.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

namespace {
using std::cout;
using std::lock_guard;
using std::make_unique;
using std::max;
using std::min;
using std::move;
using std::mutex;
using std::pair;
using std::scoped_lock;
using std::size_t;
using std::thread;
using std::uint32_t;
using std::uint64_t;
using std::unique_ptr;
using std::vector;
}

OrderBook::OrderBook(size_t pool_capacity, bool log_executions) noexcept
    : pool_(pool_capacity), log_executions_(log_executions) {
    active_orders_.reserve(pool_capacity);
}

void OrderBook::submit_order(IdType id, bool is_buy, PriceType price, PriceType qty) {
    uint32_t remaining = qty;

    if (is_buy) {
        while (remaining > 0 && !asks_.empty()) {
            auto opposite_it = asks_.begin();
            PriceLevel& level = opposite_it->second;

            if (price < level.price) {
                break;
            }

            while (remaining > 0 && level.head != nullptr) {
                Order* resting_order = level.head;
                const uint32_t trade_qty = min(remaining, resting_order->qty);
                remaining -= trade_qty;
                resting_order->qty -= trade_qty;
                level.total_volume -= trade_qty;
                ++total_executions_;

                if (log_executions_) {
                    cout << "TRADE id=" << id << " vs " << resting_order->id
                         << " qty=" << trade_qty << " price=" << level.price << '\n';
                }

                if (resting_order->qty == 0) {
                    level.erase(resting_order);
                    auto active_it = active_orders_.find(resting_order->id);
                    if (active_it != active_orders_.end()) {
                        active_orders_.erase(active_it);
                    }
                    pool_.release(resting_order);
                }

                if (level.empty()) {
                    asks_.erase(opposite_it);
                    break;
                }
            }
        }
    } else {
        while (remaining > 0 && !bids_.empty()) {
            auto opposite_it = bids_.begin();
            PriceLevel& level = opposite_it->second;

            if (price > level.price) {
                break;
            }

            while (remaining > 0 && level.head != nullptr) {
                Order* resting_order = level.head;
                const uint32_t trade_qty = min(remaining, resting_order->qty);
                remaining -= trade_qty;
                resting_order->qty -= trade_qty;
                level.total_volume -= trade_qty;
                ++total_executions_;

                if (log_executions_) {
                    cout << "TRADE id=" << id << " vs " << resting_order->id
                         << " qty=" << trade_qty << " price=" << level.price << '\n';
                }

                if (resting_order->qty == 0) {
                    level.erase(resting_order);
                    auto active_it = active_orders_.find(resting_order->id);
                    if (active_it != active_orders_.end()) {
                        active_orders_.erase(active_it);
                    }
                    pool_.release(resting_order);
                }

                if (level.empty()) {
                    bids_.erase(opposite_it);
                    break;
                }
            }
        }
    }

    if (remaining > 0) {
        Order* new_order = pool_.allocate(id, is_buy, price, remaining);
        if (new_order != nullptr) {
            insert_resting_order(new_order);
        }
    }
}

void OrderBook::cancel_order(IdType id) {
    auto active_it = active_orders_.find(id);
    if (active_it == active_orders_.end()) {
        return;
    }

    Order* order = active_it->second;
    if (order->is_buy) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            PriceLevel& level = level_it->second;
            level.erase(order);
            level.total_volume -= order->qty;
            if (level.empty()) {
                bids_.erase(level_it);
            }
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            PriceLevel& level = level_it->second;
            level.erase(order);
            level.total_volume -= order->qty;
            if (level.empty()) {
                asks_.erase(level_it);
            }
        }
    }

    active_orders_.erase(active_it);
    pool_.release(order);
}

void OrderBook::insert_resting_order(Order* order) {
    if (order->is_buy) {
        auto level_it = bids_.find(order->price);
        if (level_it == bids_.end()) {
            level_it = bids_.emplace(order->price, PriceLevel{order->price}).first;
        }

        PriceLevel& level = level_it->second;
        level.append(order);
        level.total_volume += order->qty;
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it == asks_.end()) {
            level_it = asks_.emplace(order->price, PriceLevel{order->price}).first;
        }

        PriceLevel& level = level_it->second;
        level.append(order);
        level.total_volume += order->qty;
    }

    active_orders_[order->id] = order;
}

void OrderBook::remove_resting_order(Order* order) {
    if (order->is_buy) {
        auto level_it = bids_.find(order->price);
        if (level_it == bids_.end()) {
            return;
        }

        PriceLevel& level = level_it->second;
        level.erase(order);
        level.total_volume -= order->qty;
        if (level.empty()) {
            bids_.erase(level_it);
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it == asks_.end()) {
            return;
        }

        PriceLevel& level = level_it->second;
        level.erase(order);
        level.total_volume -= order->qty;
        if (level.empty()) {
            asks_.erase(level_it);
        }
    }

    auto active_it = active_orders_.find(order->id);
    if (active_it != active_orders_.end()) {
        active_orders_.erase(active_it);
    }
    pool_.release(order);
}

size_t OrderBook::active_order_count() const noexcept {
    return active_orders_.size();
}

uint64_t OrderBook::total_executions() const noexcept {
    return total_executions_;
}

PriceLevelWorker::PriceLevelWorker(size_t pool_capacity,
                                   uint32_t partition_id,
                                   size_t worker_count,
                                   bool log_executions) noexcept
    : partition_id_(partition_id),
      worker_count_(worker_count),
      log_executions_(log_executions),
      pool_(pool_capacity) {
    active_orders_.reserve(pool_capacity / max<size_t>(1, worker_count));
}

void PriceLevelWorker::set_peers(vector<PriceLevelWorker*> peers) noexcept {
    peers_ = move(peers);
}

bool PriceLevelWorker::can_match(bool is_buy, uint32_t incoming_price, uint32_t level_price) const noexcept {
    return is_buy ? incoming_price >= level_price : incoming_price <= level_price;
}

bool PriceLevelWorker::submit_order(IdType id, bool is_buy, PriceType price, PriceType qty) {
    uint32_t remaining = qty;

    while (remaining > 0) {
        PriceLevelWorker* best_worker = nullptr;
        uint32_t best_level_price = 0;
        bool found_match = false;

        for (PriceLevelWorker* candidate : peers_) {
            if (candidate == nullptr) {
                continue;
            }

            scoped_lock lock(mutex_, candidate->mutex_);
            if (is_buy) {
                auto& opposite_levels = candidate->asks_;
                if (opposite_levels.empty()) {
                    continue;
                }

                const auto opposite_it = opposite_levels.begin();
                const PriceLevel& level = opposite_it->second;
                if (!can_match(is_buy, price, level.price)) {
                    continue;
                }

                if (!found_match || level.price > best_level_price) {
                    best_worker = candidate;
                    best_level_price = level.price;
                    found_match = true;
                }
            } else {
                auto& opposite_levels = candidate->bids_;
                if (opposite_levels.empty()) {
                    continue;
                }

                const auto opposite_it = opposite_levels.begin();
                const PriceLevel& level = opposite_it->second;
                if (!can_match(is_buy, price, level.price)) {
                    continue;
                }

                if (!found_match || level.price < best_level_price) {
                    best_worker = candidate;
                    best_level_price = level.price;
                    found_match = true;
                }
            }
        }

        if (!found_match) {
            break;
        }

        scoped_lock lock(mutex_, best_worker->mutex_);
        if (is_buy) {
            auto& opposite_levels = best_worker->asks_;
            if (opposite_levels.empty()) {
                continue;
            }

            auto opposite_it = opposite_levels.begin();
            PriceLevel& level = opposite_it->second;
            if (level.head == nullptr) {
                continue;
            }

            Order* resting_order = level.head;
            const uint32_t trade_qty = min(remaining, resting_order->qty);
            remaining -= trade_qty;
            resting_order->qty -= trade_qty;
            level.total_volume -= trade_qty;
            ++total_executions_;

            if (log_executions_) {
                cout << "TRADE id=" << id << " vs " << resting_order->id
                     << " qty=" << trade_qty << " price=" << level.price << '\n';
            }

            if (resting_order->qty == 0) {
                level.erase(resting_order);
                auto active_it = active_orders_.find(resting_order->id);
                if (active_it != active_orders_.end()) {
                    active_orders_.erase(active_it);
                }
                best_worker->pool_.release(resting_order);
            }

            if (level.empty()) {
                opposite_levels.erase(opposite_it);
            }
        } else {
            auto& opposite_levels = best_worker->bids_;
            if (opposite_levels.empty()) {
                continue;
            }

            auto opposite_it = opposite_levels.begin();
            PriceLevel& level = opposite_it->second;
            if (level.head == nullptr) {
                continue;
            }

            Order* resting_order = level.head;
            const uint32_t trade_qty = min(remaining, resting_order->qty);
            remaining -= trade_qty;
            resting_order->qty -= trade_qty;
            level.total_volume -= trade_qty;
            ++total_executions_;

            if (log_executions_) {
                cout << "TRADE id=" << id << " vs " << resting_order->id
                     << " qty=" << trade_qty << " price=" << level.price << '\n';
            }

            if (resting_order->qty == 0) {
                level.erase(resting_order);
                auto active_it = active_orders_.find(resting_order->id);
                if (active_it != active_orders_.end()) {
                    active_orders_.erase(active_it);
                }
                best_worker->pool_.release(resting_order);
            }

            if (level.empty()) {
                opposite_levels.erase(opposite_it);
            }
        }
    }

    if (remaining > 0) {
        Order* new_order = pool_.allocate(id, is_buy, price, remaining);
        if (new_order != nullptr) {
            insert_resting_order(new_order);
        }
    }

    return true;
}

void PriceLevelWorker::cancel_order(IdType id) {
    lock_guard<mutex> lock(mutex_);
    auto active_it = active_orders_.find(id);
    if (active_it == active_orders_.end()) {
        return;
    }

    Order* order = active_it->second;
    if (order->is_buy) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            PriceLevel& level = level_it->second;
            level.erase(order);
            level.total_volume -= order->qty;
            if (level.empty()) {
                bids_.erase(level_it);
            }
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            PriceLevel& level = level_it->second;
            level.erase(order);
            level.total_volume -= order->qty;
            if (level.empty()) {
                asks_.erase(level_it);
            }
        }
    }

    active_orders_.erase(active_it);
    pool_.release(order);
}

void PriceLevelWorker::insert_resting_order(Order* order) {
    lock_guard<mutex> lock(mutex_);
    if (order->is_buy) {
        auto level_it = bids_.find(order->price);
        if (level_it == bids_.end()) {
            level_it = bids_.emplace(order->price, PriceLevel{order->price}).first;
        }
        PriceLevel& level = level_it->second;
        level.append(order);
        level.total_volume += order->qty;
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it == asks_.end()) {
            level_it = asks_.emplace(order->price, PriceLevel{order->price}).first;
        }
        PriceLevel& level = level_it->second;
        level.append(order);
        level.total_volume += order->qty;
    }

    active_orders_[order->id] = order;
}

void PriceLevelWorker::remove_resting_order(Order* order) {
    lock_guard<mutex> lock(mutex_);
    if (order->is_buy) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            PriceLevel& level = level_it->second;
            level.erase(order);
            level.total_volume -= order->qty;
            if (level.empty()) {
                bids_.erase(level_it);
            }
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            PriceLevel& level = level_it->second;
            level.erase(order);
            level.total_volume -= order->qty;
            if (level.empty()) {
                asks_.erase(level_it);
            }
        }
    }

    auto active_it = active_orders_.find(order->id);
    if (active_it != active_orders_.end()) {
        active_orders_.erase(active_it);
    }
    pool_.release(order);
}

size_t PriceLevelWorker::active_order_count() const noexcept {
    lock_guard<mutex> lock(mutex_);
    return active_orders_.size();
}

uint64_t PriceLevelWorker::total_executions() const noexcept {
    lock_guard<mutex> lock(mutex_);
    return total_executions_;
}

MultiThreadedOrderBook::MultiThreadedOrderBook(size_t pool_capacity,
                                               size_t worker_count,
                                               size_t batch_size) noexcept
    : worker_count_(max<size_t>(1, worker_count)),
      batch_size_(batch_size),
      workers_() {
    workers_.reserve(worker_count_);
    for (size_t i = 0; i < worker_count_; ++i) {
        workers_.emplace_back(make_unique<PriceLevelWorker>(
            pool_capacity / max<size_t>(1, worker_count_),
            static_cast<uint32_t>(i),
            worker_count_,
            false));
    }

    vector<PriceLevelWorker*> peers;
    peers.reserve(worker_count_ - 1);
    for (size_t i = 0; i < workers_.size(); ++i) {
        peers.clear();
        for (size_t j = 0; j < workers_.size(); ++j) {
            if (i != j) {
                peers.push_back(workers_[j].get());
            }
        }
        workers_[i]->set_peers(peers);
    }
}

void MultiThreadedOrderBook::submit_batch(const vector<OrderCommand>& commands) {
    if (commands.empty()) {
        return;
    }

    const size_t worker_count = min(worker_count_, commands.size());
    vector<thread> threads;
    threads.reserve(worker_count);

    const size_t chunk = (commands.size() + worker_count - 1) / worker_count;
    for (size_t worker = 0; worker < worker_count; ++worker) {
        const size_t begin = worker * chunk;
        const size_t end = min(begin + chunk, commands.size());
        if (begin >= end) {
            break;
        }

        threads.emplace_back([this, &commands, begin, end]() {
            vector<pair<size_t, OrderCommand>> local_batch;
            local_batch.reserve(batch_size_);
            for (size_t i = begin; i < end; ++i) {
                const OrderCommand& command = commands[i];
                const size_t partition = static_cast<size_t>(command.price % worker_count_);
                local_batch.emplace_back(partition, command);
                if (local_batch.size() == batch_size_) {
                    for (const auto& batch_entry : local_batch) {
                        workers_[batch_entry.first]->submit_order(batch_entry.second.id, batch_entry.second.is_buy, batch_entry.second.price, batch_entry.second.qty);
                    }
                    local_batch.clear();
                }
            }

            for (const auto& batch_entry : local_batch) {
                workers_[batch_entry.first]->submit_order(batch_entry.second.id, batch_entry.second.is_buy, batch_entry.second.price, batch_entry.second.qty);
            }
        });
    }

    for (thread& thread : threads) {
        thread.join();
    }
}

size_t MultiThreadedOrderBook::active_order_count() const noexcept {
    size_t total = 0;
    for (const auto& worker : workers_) {
        total += worker->active_order_count();
    }
    return total;
}

uint64_t MultiThreadedOrderBook::total_executions() const noexcept {
    uint64_t total = 0;
    for (const auto& worker : workers_) {
        total += worker->total_executions();
    }
    return total;
}
