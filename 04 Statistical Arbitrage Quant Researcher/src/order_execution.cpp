// CRITICAL FIX: Define NOMINMAX before any Windows header to prevent min/max macro hijacking
#if defined(_WIN32) || defined(_WIN64)
    #define NOMINMAX
#endif

#include "order_execution.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm> // For std::min

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>  // CRITICAL FIX: Contains Windows implementation for inet_pton
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

LiveExchangeInterface::LiveExchangeInterface() : socket_fd(-1), is_connected(false) {}

LiveExchangeInterface::~LiveExchangeInterface() {
    if (socket_fd != -1) {
#if defined(_WIN32) || defined(_WIN64)
        closesocket(socket_fd);
#else
        close(socket_fd);
#endif
    }
}

bool LiveExchangeInterface::connect_to_exchange(const std::string& host, uint16_t port) {
#if defined(_WIN32) || defined(_WIN64)
    // Initialize Windows Sockets Subsystem
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) return false;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Works perfectly on Windows now thanks to <ws2tcpip.h>
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        return false;
    }

    if (connect(socket_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        is_connected = false;
        return false;
    }

    is_connected = true;
    return true;
}

std::vector<uint8_t> LiveExchangeInterface::serialize_ouch_binary(const TargetOrder& order) {
    OuchEnterOrderMessage msg;
    
    std::memset(msg.order_token, ' ', sizeof(msg.order_token));
    std::memcpy(msg.order_token, order.cl_ord_id.c_str(), std::min(order.cl_ord_id.length(), size_t(14)));
    
    msg.buy_sell_indicator = order.is_buy ? 'B' : 'S';
    msg.shares = htonl(order.quantity);
    msg.price = htonl(static_cast<uint32_t>(order.limit_price * 10000));
    
    std::memset(msg.stock, ' ', sizeof(msg.stock));
    std::memcpy(msg.stock, order.ticker.c_str(), std::min(order.ticker.length(), size_t(8)));

    uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&msg);
    return std::vector<uint8_t>(byte_ptr, byte_ptr + sizeof(OuchEnterOrderMessage));
}

std::vector<uint8_t> LiveExchangeInterface::serialize_fix_tag_value(const TargetOrder& order) {
    std::stringstream body;
    body << "35=D\001"
         << "11=" << order.cl_ord_id << "\001"
         << "55=" << order.ticker << "\001"
         << "54=" << (order.is_buy ? "1" : "2") << "\001"
         << "38=" << order.quantity << "\001"
         << "44=" << std::fixed << std::setprecision(4) << order.limit_price << "\001";

    std::stringstream header;
    header << "8=FIX.4.2\001"
           << "9=" << body.str().length() << "\001"
           << body.str();

    std::string full_msg = header.str();
    return std::vector<uint8_t>(full_msg.begin(), full_msg.end());
}

void LiveExchangeInterface::send_order_async(const TargetOrder& order) {
    if (!is_connected) return;
    std::vector<uint8_t> payload = serialize_ouch_binary(order);
    send(socket_fd, reinterpret_cast<const char*>(payload.data()), static_cast<int>(payload.size()), 0);
}

void LiveExchangeInterface::process_inbound_responses() {
    if (!is_connected) return;

    char buffer[256];
#if defined(_WIN32) || defined(_WIN64)
    // Non-blocking socket flag mapping for Windows systems
    u_long mode = 1;
    ioctlsocket(socket_fd, FIONBIO, &mode);
    int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
#else
    int bytes_received = recv(socket_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
#endif

    if (bytes_received > 0) {
        // Handle exchange execution confirmations...
    }
}
