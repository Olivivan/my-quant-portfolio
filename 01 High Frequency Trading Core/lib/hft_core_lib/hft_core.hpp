#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_set>

/**
 * @brief Core matching primitives for the HFT portfolio engine.
 */
namespace HFT {
    /**
     * @brief Immutable order payload accepted by the engine APIs.
     */
    struct Order {
        /** Unique positive order identifier. */
        uint64_t id;
        /** Positive limit price. */
        double price;
        /** Positive remaining quantity in units. */
        uint32_t quantity;
        /** True for buy orders, false for sell orders. */
        bool isBuy;
        /** Non-negative event timestamp used for time-priority ordering. */
        long timestamp;
    };

    /**
     * @brief Deterministic in-memory limit order book implementing price-time priority.
     */
    class HFTCore {
    public:
        /**
         * @brief Reserved engine update hook.
         *
         * The current portfolio implementation is fully event-driven and
         * performs all work during order operations.
         */
        void Update() {}

        /**
         * @brief Submit a limit order and attempt immediate matching.
         * @param order Incoming order.
         * @return true if accepted, false when validation fails or id is already active.
         */
        bool LimitOrder(Order order) {
            if (!ValidateOrder(order) || HasOrderId(order.id)) {
                return false;
            }

            if (order.isBuy) {
                MatchIncomingBuy(order);
                if (order.quantity > 0) {
                    InsertRestingOrder(m_BuyBook, order);
                }
            } else {
                MatchIncomingSell(order);
                if (order.quantity > 0) {
                    InsertRestingOrder(m_SellBook, order);
                }
            }

            return true;
        }

        /**
         * @brief Cancel an active order by id.
         * @param orderId Active order identifier.
         * @return true if an order was found and canceled.
         */
        bool CancelOrder(uint64_t orderId) {
            if (EraseOrderFromBook(m_BuyBook, orderId)) {
                m_ActiveOrderIds.erase(orderId);
                return true;
            }

            if (EraseOrderFromBook(m_SellBook, orderId)) {
                m_ActiveOrderIds.erase(orderId);
                return true;
            }

            return false;
        }

        /**
         * @brief Replace an existing active order with a new payload.
         * @param orderId Existing active order id.
         * @param newPrice Replacement price.
         * @param newQuantity Replacement quantity.
         * @param newTimestamp Replacement timestamp.
         * @return true if replacement succeeds, false otherwise.
         */
        bool ReplaceOrder(uint64_t orderId, double newPrice, uint32_t newQuantity, long newTimestamp) {
            std::optional<Order> existing = RemoveOrderById(orderId);
            if (!existing.has_value()) {
                return false;
            }

            Order replacement = existing.value();
            replacement.price = newPrice;
            replacement.quantity = newQuantity;
            replacement.timestamp = newTimestamp;

            if (!ValidateOrder(replacement)) {
                // Reinsert original order if replacement payload is invalid.
                if (replacement.isBuy) {
                    InsertRestingOrder(m_BuyBook, existing.value());
                } else {
                    InsertRestingOrder(m_SellBook, existing.value());
                }
                return false;
            }

            if (replacement.isBuy) {
                MatchIncomingBuy(replacement);
                if (replacement.quantity > 0) {
                    InsertRestingOrder(m_BuyBook, replacement);
                }
            } else {
                MatchIncomingSell(replacement);
                if (replacement.quantity > 0) {
                    InsertRestingOrder(m_SellBook, replacement);
                }
            }

            return true;
        }

        /**
         * @brief Get buy-side book levels sorted from best bid to worst bid.
         */
        const std::map<double, std::list<Order>, std::greater<double>>& GetBuyBook() const {
            return m_BuyBook;
        }

        /**
         * @brief Get sell-side book levels sorted from best ask to worst ask.
         */
        const std::map<double, std::list<Order>>& GetSellBook() const {
            return m_SellBook;
        }

        /**
         * @brief Best bid price, if present.
         */
        std::optional<double> GetBestBidPrice() const {
            if (m_BuyBook.empty()) {
                return std::nullopt;
            }
            return m_BuyBook.begin()->first;
        }

        /**
         * @brief Best ask price, if present.
         */
        std::optional<double> GetBestAskPrice() const {
            if (m_SellBook.empty()) {
                return std::nullopt;
            }
            return m_SellBook.begin()->first;
        }

    private:
        // Price-Time Priority: Map keys handle Price, List handles Time
        std::map<double, std::list<Order>, std::greater<double>> m_BuyBook;
        std::map<double, std::list<Order>> m_SellBook;
        std::unordered_set<uint64_t> m_ActiveOrderIds;

        bool ValidateOrder(const Order& order) const {
            return order.id > 0 &&
                   order.quantity > 0 &&
                   order.timestamp >= 0 &&
                   std::isfinite(order.price) &&
                   order.price > 0.0;
        }

        bool HasOrderId(uint64_t orderId) const {
            return m_ActiveOrderIds.find(orderId) != m_ActiveOrderIds.end();
        }

        template <typename BookType>
        void InsertRestingOrder(BookType& book, const Order& order) {
            auto& ordersAtPrice = book[order.price];

            // Keep FIFO for equal timestamp while allowing out-of-order ingestion.
            auto it = ordersAtPrice.begin();
            while (it != ordersAtPrice.end() && it->timestamp <= order.timestamp) {
                ++it;
            }
            ordersAtPrice.insert(it, order);
            m_ActiveOrderIds.insert(order.id);
        }

        template <typename BookType>
        bool EraseOrderFromBook(BookType& book, uint64_t orderId) {
            for (auto priceIt = book.begin(); priceIt != book.end(); ++priceIt) {
                auto& orders = priceIt->second;
                for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
                    if (orderIt->id == orderId) {
                        orders.erase(orderIt);
                        if (orders.empty()) {
                            book.erase(priceIt);
                        }
                        return true;
                    }
                }
            }
            return false;
        }

        std::optional<Order> RemoveOrderById(uint64_t orderId) {
            for (auto priceIt = m_BuyBook.begin(); priceIt != m_BuyBook.end(); ++priceIt) {
                auto& orders = priceIt->second;
                for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
                    if (orderIt->id == orderId) {
                        Order found = *orderIt;
                        orders.erase(orderIt);
                        if (orders.empty()) {
                            m_BuyBook.erase(priceIt);
                        }
                        m_ActiveOrderIds.erase(orderId);
                        return found;
                    }
                }
            }

            for (auto priceIt = m_SellBook.begin(); priceIt != m_SellBook.end(); ++priceIt) {
                auto& orders = priceIt->second;
                for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
                    if (orderIt->id == orderId) {
                        Order found = *orderIt;
                        orders.erase(orderIt);
                        if (orders.empty()) {
                            m_SellBook.erase(priceIt);
                        }
                        m_ActiveOrderIds.erase(orderId);
                        return found;
                    }
                }
            }

            return std::nullopt;
        }

        void MatchIncomingBuy(Order& incoming) {
            while (incoming.quantity > 0 && !m_SellBook.empty()) {
                auto sellLevelIt = m_SellBook.begin();
                if (incoming.price < sellLevelIt->first) {
                    break;
                }

                auto& sellOrders = sellLevelIt->second;
                while (incoming.quantity > 0 && !sellOrders.empty()) {
                    auto& resting = sellOrders.front();
                    const uint32_t fillQty = (incoming.quantity < resting.quantity) ? incoming.quantity : resting.quantity;
                    incoming.quantity -= fillQty;
                    resting.quantity -= fillQty;

                    if (resting.quantity == 0) {
                        m_ActiveOrderIds.erase(resting.id);
                        sellOrders.pop_front();
                    }
                }

                if (sellOrders.empty()) {
                    m_SellBook.erase(sellLevelIt);
                }
            }
        }

        void MatchIncomingSell(Order& incoming) {
            while (incoming.quantity > 0 && !m_BuyBook.empty()) {
                auto buyLevelIt = m_BuyBook.begin();
                if (incoming.price > buyLevelIt->first) {
                    break;
                }

                auto& buyOrders = buyLevelIt->second;
                while (incoming.quantity > 0 && !buyOrders.empty()) {
                    auto& resting = buyOrders.front();
                    const uint32_t fillQty = (incoming.quantity < resting.quantity) ? incoming.quantity : resting.quantity;
                    incoming.quantity -= fillQty;
                    resting.quantity -= fillQty;

                    if (resting.quantity == 0) {
                        m_ActiveOrderIds.erase(resting.id);
                        buyOrders.pop_front();
                    }
                }

                if (buyOrders.empty()) {
                    m_BuyBook.erase(buyLevelIt);
                }
            }
        }
    };
}