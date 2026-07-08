#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace ull::network {

enum class TimestampingMode : std::uint8_t {
    disabled,
    software,
    hardware,
};

class UdpReceiver {
public:
    UdpReceiver() = default;

    [[nodiscard]] std::size_t receive(std::span<char> buffer) noexcept;
    [[nodiscard]] static bool hardware_timestamping_supported() noexcept;
    [[nodiscard]] bool configure_timestamping(int socket_fd, TimestampingMode mode) noexcept;

    [[nodiscard]] TimestampingMode timestamping_mode() const noexcept {
        return timestamping_mode_;
    }

private:
    TimestampingMode timestamping_mode_{TimestampingMode::disabled};
};

} // namespace ull::network
