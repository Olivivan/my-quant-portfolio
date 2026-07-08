#include "ull/network/kernel_bypass.hpp"

#if defined(ULL_PLATFORM_LINUX) && (ULL_PLATFORM_LINUX == 1) && defined(ULL_ENABLE_EFVI) && (ULL_ENABLE_EFVI == 1)
#if __has_include(<etherfabric/vi.h>) && __has_include(<onload/extensions.h>)
#define ULL_HAVE_EFVI_HEADERS 1
#include <etherfabric/vi.h>
#include <onload/extensions.h>
#else
#define ULL_HAVE_EFVI_HEADERS 0
#endif
#else
#define ULL_HAVE_EFVI_HEADERS 0
#endif

namespace ull::network {

bool KernelBypassReceiver::runtime_supported() noexcept {
#if ULL_HAVE_EFVI_HEADERS
    return true;
#else
    return false;
#endif
}

bool KernelBypassReceiver::initialize(std::string_view interface_name, int receive_queue_id) noexcept {
    if (interface_name.empty() || receive_queue_id < 0) [[unlikely]] {
        active_ = false;
        backend_ = KernelBypassBackend::disabled;
        return false;
    }

#if ULL_HAVE_EFVI_HEADERS
    // Full ef_vi resource setup (PD/VI/memreg/eventq/rxq) is done in deployment-specific bootstrap.
    // This integration point confirms headers/platform are available and switches pipeline to bypass mode.
    active_ = true;
    backend_ = KernelBypassBackend::solarflare_efvi;
    return true;
#else
    (void)interface_name;
    (void)receive_queue_id;
    active_ = false;
    backend_ = KernelBypassBackend::disabled;
    return false;
#endif
}

std::size_t KernelBypassReceiver::poll(std::span<std::byte> destination) noexcept {
    if (!active_ || destination.empty()) [[unlikely]] {
        return 0U;
    }

    // Polling ring consumption is integrated once queue resources are connected.
    return 0U;
}

} // namespace ull::network
