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

std::optional<std::string_view> DataAccessView::get_string(FeedTag tag) const noexcept {
    return value_for(tag);
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

std::optional<std::uint32_t> DataAccessView::get_uint32(FeedTag tag) const noexcept {
    const auto value = value_for(tag);
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

std::optional<double> DataAccessView::get_double(FeedTag tag) const noexcept {
    const auto value = value_for(tag);
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

    if (const auto known_tag = lookup_feed_tag(key); known_tag.has_value()) {
        return value_for(*known_tag);
    }

    constexpr std::size_t mask = StructuralScanResult::index_capacity - 1;
    std::size_t slot = static_cast<std::size_t>(structural_key_hash(key)) & mask;

    for (std::size_t probe = 0; probe < StructuralScanResult::index_capacity; ++probe) {
        const std::uint8_t entry = scan_.index_slots[slot];
        if (entry == 0U) {
            return std::nullopt;
        }

        const auto& field = scan_.fields[static_cast<std::size_t>(entry - 1U)];
        const auto field_key = payload_.substr(field.key_offset, field.key_length);
        if (field_key == key) {
            return payload_.substr(field.value_offset, field.value_length);
        }

        slot = (slot + 1) & mask;
    }

    return std::nullopt;
}

std::optional<std::string_view> DataAccessView::value_for(FeedTag tag) const noexcept {
    if (!scan_.valid) {
        return std::nullopt;
    }

    const auto tag_index = static_cast<std::size_t>(tag);
    if (tag_index >= feed_tag_count) {
        return std::nullopt;
    }

    const std::uint8_t entry = scan_.known_tag_slots[tag_index];
    if (entry == 0U) {
        return std::nullopt;
    }

    const auto& field = scan_.fields[static_cast<std::size_t>(entry - 1U)];
    return payload_.substr(field.value_offset, field.value_length);
}

} // namespace ull::feeds
