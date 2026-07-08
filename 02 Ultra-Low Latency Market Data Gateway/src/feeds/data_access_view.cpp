#include "ull/feeds/data_access_view.hpp"

#include <charconv>
#include <system_error>

namespace ull::feeds {

DataAccessView::DataAccessView(std::string_view payload, StructuralScanResult scan) noexcept
    : payload_(payload),
      scan_(scan) {}

bool DataAccessView::valid() const noexcept {
    return scan_.valid;
}

std::optional<std::string_view> DataAccessView::get_string(std::string_view key) const noexcept {
    return value_for(key);
}

std::optional<std::uint32_t> DataAccessView::get_uint32(std::string_view key) const noexcept {
    const auto value = value_for(key);
    if (!value.has_value()) {
        return std::nullopt;
    }

    std::uint32_t out = 0;
    const auto* begin = value->data();
    const auto* end = begin + value->size();
    const auto parsed = std::from_chars(begin, end, out);
    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        return std::nullopt;
    }

    return out;
}

std::optional<double> DataAccessView::get_double(std::string_view key) const noexcept {
    const auto value = value_for(key);
    if (!value.has_value()) {
        return std::nullopt;
    }

    double out = 0.0;
    const auto* begin = value->data();
    const auto* end = begin + value->size();
    const auto parsed = std::from_chars(begin, end, out);
    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        return std::nullopt;
    }

    return out;
}

std::optional<std::string_view> DataAccessView::value_for(std::string_view key) const noexcept {
    if (!scan_.valid || key.empty()) {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < scan_.field_count; ++i) {
        const auto& field = scan_.fields[i];
        const auto field_key = payload_.substr(field.key_offset, field.key_length);
        if (field_key == key) {
            return payload_.substr(field.value_offset, field.value_length);
        }
    }

    return std::nullopt;
}

} // namespace ull::feeds
