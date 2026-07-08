#include "ull/network/kernel_bypass.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Kernel bypass runtime support reflects platform/header availability", "[network][efvi]") {
#if defined(__linux__) && defined(ULL_ENABLE_EFVI) && (ULL_ENABLE_EFVI == 1)
    // On Linux with ef_vi integration enabled, runtime availability depends on local headers/toolchain.
    SUCCEED();
#else
    REQUIRE_FALSE(ull::network::KernelBypassReceiver::runtime_supported());
#endif
}

TEST_CASE("Kernel bypass rejects invalid bootstrap parameters", "[network][efvi]") {
    ull::network::KernelBypassReceiver receiver;

    REQUIRE_FALSE(receiver.initialize("", 0));
    REQUIRE_FALSE(receiver.initialize("eth0", -1));
    REQUIRE_FALSE(receiver.active());
    REQUIRE(receiver.backend() == ull::network::KernelBypassBackend::disabled);
}
