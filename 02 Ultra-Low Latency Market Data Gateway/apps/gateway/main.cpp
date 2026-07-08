#include "ull/gateway/market_data_gateway.hpp"

int main() {
    ull::gateway::MarketDataGateway gateway;
    gateway.start();
    gateway.stop();
    return gateway.is_running() ? 1 : 0;
}
