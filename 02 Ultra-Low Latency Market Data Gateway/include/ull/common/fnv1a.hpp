#pragma once

#include <cstdint>
#include <string_view>

namespace ull::common {

inline constexpr std::uint64_t fnv1a_offset_basis_64 = 14695981039346656037ULL;
inline constexpr std::uint64_t fnv1a_prime_64 = 1099511628211ULL;

consteval std::uint64_t fnv1a_64_literal(const char* text, std::size_t len) {
    std::uint64_t hash = fnv1a_offset_basis_64;
    for (std::size_t i = 0; i < len; ++i) {
        hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(text[i]));
        hash *= fnv1a_prime_64;
    }

    return hash;
}

constexpr std::uint64_t fnv1a_64(std::string_view text) noexcept {
    std::uint64_t hash = fnv1a_offset_basis_64;
    for (const unsigned char ch : text) {
        hash ^= static_cast<std::uint64_t>(ch);
        hash *= fnv1a_prime_64;
    }

    return hash;
}

} // namespace ull::common
