#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace ull::network {

enum class KernelBypassBackend : std::uint8_t {
    disabled,
    solarflare_efvi,
};

class KernelBypassReceiver {
public:
    [[nodiscard]] static constexpr bool compile_time_enabled() noexcept {
#if defined(ULL_ENABLE_EFVI) && (ULL_ENABLE_EFVI == 1)
        return true;
#else
        return false;
#endif
    }

    [[nodiscard]] static bool runtime_supported() noexcept;

    [[nodiscard]] bool initialize(std::string_view interface_name, int receive_queue_id) noexcept;
    [[nodiscard]] std::size_t poll(std::span<std::byte> destination) noexcept;

    [[nodiscard]] bool active() const noexcept {
        return active_;
    }

    [[nodiscard]] KernelBypassBackend backend() const noexcept {
        return backend_;
    }

private:
    bool active_{false};
    KernelBypassBackend backend_{KernelBypassBackend::disabled};
};

} // namespace ull::network
