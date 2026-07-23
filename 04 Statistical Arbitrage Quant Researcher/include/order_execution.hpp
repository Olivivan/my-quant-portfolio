#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

// Ultra low-latency hardware-aligned binary byte alignment pack definitions
#pragma pack(push, 1)
struct OuchEnterOrderMessage {
    char type = 'O';                         // OUCH Enter Order token flag identification
    char order_token[14];                    // Distinct ID mapping pointer
    char buy_sell_indicator;                 // 'B' = Buy, 'S' = Sell
    uint32_t shares;                         // Quantity internal mask
    uint32_t price;                          // Scaled integer (e.g. Price * 10000)
    char stock[8];                           // Right-padded equity ticker string
};
#pragma pack(pop)

struct TargetOrder {
    std::string cl_ord_id;
    std::string ticker;
    bool is_buy;
    uint32_t quantity;
    double limit_price;
};

class IBrokerConnectivity {
public:
    virtual ~IBrokerConnectivity() = default;
    virtual bool connect_to_exchange(const std::string& host, uint16_t port) = 0;
    virtual void send_order_async(const TargetOrder& order) = 0;
    virtual void process_inbound_responses() = 0;
};

class LiveExchangeInterface : public IBrokerConnectivity {
public:
    LiveExchangeInterface();
    ~LiveExchangeInterface() override;

    bool connect_to_exchange(const std::string& host, uint16_t port) override;
    void send_order_async(const TargetOrder& order) override;
    void process_inbound_responses() override;

private:
    int socket_fd;
    bool is_connected;
    
    // Core binary serialization pipelines
    std::vector<uint8_t> serialize_fix_tag_value(const TargetOrder& order);
    std::vector<uint8_t> serialize_ouch_binary(const TargetOrder& order);
};
