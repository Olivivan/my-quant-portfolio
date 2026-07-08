#pragma once

#include "ull/feeds/structural_scan.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace ull::feeds {

class DataAccessView {
public:
    DataAccessView(std::string_view payload, StructuralScanResult scan) noexcept;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::optional<std::string_view> get_string(std::string_view key) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> get_uint32(std::string_view key) const noexcept;
    [[nodiscard]] std::optional<double> get_double(std::string_view key) const noexcept;

private:
    [[nodiscard]] std::optional<std::string_view> value_for(std::string_view key) const noexcept;

    std::string_view payload_{};
    StructuralScanResult scan_{};
};

} // namespace ull::feeds
