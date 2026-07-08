#include "ull/network/udp_receiver.hpp"

#if defined(__linux__)
#include <linux/net_tstamp.h>
#include <sys/socket.h>
#endif

namespace ull::network {

std::size_t UdpReceiver::receive(std::span<char> buffer) noexcept {
    // Stub for baseline scaffold; real socket ingestion is implemented in later tasks.
    return buffer.empty() ? 0U : 0U;
}

bool UdpReceiver::hardware_timestamping_supported() noexcept {
#if defined(__linux__)
    return true;
#else
    return false;
#endif
}

bool UdpReceiver::configure_timestamping(int socket_fd, TimestampingMode mode) noexcept {
#if defined(__linux__)
    if (socket_fd < 0) [[unlikely]] {
        return false;
    }

    if (mode == TimestampingMode::disabled) {
        int off = 0;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_TIMESTAMPING, &off, sizeof(off)) != 0) {
            return false;
        }

        timestamping_mode_ = TimestampingMode::disabled;
        return true;
    }

    int flags = 0;
    if (mode == TimestampingMode::software) {
        flags = SOF_TIMESTAMPING_RX_SOFTWARE |
                SOF_TIMESTAMPING_SOFTWARE;
    } else if (mode == TimestampingMode::hardware) {
        flags = SOF_TIMESTAMPING_RX_HARDWARE |
                SOF_TIMESTAMPING_RAW_HARDWARE |
                SOF_TIMESTAMPING_SOFTWARE;
    } else {
        return false;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof(flags)) != 0) {
        return false;
    }

    timestamping_mode_ = mode;
    return true;
#else
    (void)socket_fd;
    (void)mode;
    timestamping_mode_ = TimestampingMode::disabled;
    return false;
#endif
}

} // namespace ull::network
