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
                    m_BuyBook[order.price].push_back(order);
                }
            } else {
                MatchIncomingSell(order);
                if (order.quantity > 0) {
                    m_SellBook[order.price].push_back(order);
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