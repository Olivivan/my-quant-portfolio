#pragma once

#include <cstddef>
#include <span>

namespace ull::network {

class UdpReceiver {
public:
    UdpReceiver() = default;

    [[nodiscard]] std::size_t receive(std::span<char> buffer) noexcept;
};

} // namespace ull::network
