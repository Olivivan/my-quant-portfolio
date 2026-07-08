#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <map>

using namespace std;

namespace HFT {
    struct Order {
        uint64_t id;
        double price;
        uint32_t quantity;
        bool isBuy;
        long timestamp;
    };

    class HFTCore {
    public:
        void Update() {}

        void LimitOrder(Order order) {
            if (order.quantity == 0) {
                return;
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
        }

        const map<double, list<Order>, greater<double>>& GetBuyBook() const {
            return m_BuyBook;
        }

        const map<double, list<Order>>& GetSellBook() const {
            return m_SellBook;
        }

    private:
        // Price-Time Priority: Map keys handle Price, List handles Time
        map<double, list<Order>, greater<double>> m_BuyBook;
        map<double, list<Order>> m_SellBook;

        template <typename BookType>
        void InsertRestingOrder(BookType& book, const Order& order) {
            auto& ordersAtPrice = book[order.price];

            // Keep FIFO for equal timestamp while allowing out-of-order ingestion.
            auto it = ordersAtPrice.begin();
            while (it != ordersAtPrice.end() && it->timestamp <= order.timestamp) {
                ++it;
            }
            ordersAtPrice.insert(it, order);
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