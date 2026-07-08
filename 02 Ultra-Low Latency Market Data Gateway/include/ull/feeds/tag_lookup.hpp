#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#if defined(__cpp_consteval) && (__cpp_consteval >= 201811L)
#define ULL_CONSTEVAL consteval
#else
#define ULL_CONSTEVAL constexpr
#endif

namespace ull::feeds {

enum class FeedTag : std::uint8_t {
    sym = 0,
    px = 1,
    qty = 2,
    venue = 3,
    seq = 4,
    unknown = 5,
};

inline constexpr std::size_t feed_tag_count = 5;

struct TagLookupEntry {
    std::string_view name;
    FeedTag tag;
};

template <std::size_t N>
ULL_CONSTEVAL std::array<TagLookupEntry, N> build_sorted_tag_table(std::array<TagLookupEntry, N> entries) {
    for (std::size_t i = 1; i < N; ++i) {
        auto current = entries[i];
        std::size_t j = i;
        while (j > 0 && entries[j - 1].name > current.name) {
            entries[j] = entries[j - 1];
            --j;
        }

        entries[j] = current;
    }

    return entries;
}

inline constexpr auto feed_tag_lookup_table = build_sorted_tag_table<feed_tag_count>({{{"venue", FeedTag::venue},
                                                                                        {"sym", FeedTag::sym},
                                                                                        {"qty", FeedTag::qty},
                                                                                        {"seq", FeedTag::seq},
                                                                                        {"px", FeedTag::px}}});

inline constexpr std::array<std::string_view, feed_tag_count> feed_tag_names = {
    "sym",
    "px",
    "qty",
    "venue",
    "seq",
};

[[nodiscard]] constexpr std::optional<FeedTag> lookup_feed_tag(std::string_view name) noexcept {
    std::size_t first = 0;
    std::size_t count = feed_tag_lookup_table.size();

    while (count > 0) {
        const std::size_t step = count / 2;
        const std::size_t mid = first + step;
        if (feed_tag_lookup_table[mid].name < name) {
            first = mid + 1;
            count -= step + 1;
        } else {
            count = step;
        }
    }

    if (first < feed_tag_lookup_table.size() && feed_tag_lookup_table[first].name == name) {
        return feed_tag_lookup_table[first].tag;
    }

    return std::nullopt;
}

[[nodiscard]] constexpr std::string_view feed_tag_name(FeedTag tag) noexcept {
    const auto idx = static_cast<std::size_t>(tag);
    if (idx >= feed_tag_count) {
        return {};
    }

    return feed_tag_names[idx];
}

} // namespace ull::feeds

#undef ULL_CONSTEVAL
