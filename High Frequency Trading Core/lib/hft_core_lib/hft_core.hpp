#pragma once
#include <vector>

namespace HFT {
    class HFTCore {
    public:
        void Update();
        void AddOrder(double price, int quantity, bool isBuy);
    private:
        // Order book data structures
    };
}