#include "ull/network/udp_receiver.hpp"

namespace ull::network {

std::size_t UdpReceiver::receive(std::span<char> buffer) noexcept {
    // Stub for baseline scaffold; real socket ingestion is implemented in later tasks.
    return buffer.empty() ? 0U : 0U;
}

} // namespace ull::network
