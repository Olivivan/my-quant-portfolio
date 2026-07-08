#include "ull/network/udp_receiver.hpp"

#include <catch2/catch.hpp>

TEST_CASE("UdpReceiver reports hardware timestamping support per platform", "[network][timestamping]") {
#if defined(__linux__)
    REQUIRE(ull::network::UdpReceiver::hardware_timestamping_supported());
#else
    REQUIRE_FALSE(ull::network::UdpReceiver::hardware_timestamping_supported());
#endif
}

TEST_CASE("UdpReceiver rejects invalid socket configuration requests", "[network][timestamping]") {
    ull::network::UdpReceiver receiver;

    const bool configured = receiver.configure_timestamping(-1, ull::network::TimestampingMode::hardware);
    REQUIRE_FALSE(configured);
    REQUIRE(receiver.timestamping_mode() == ull::network::TimestampingMode::disabled);
}
