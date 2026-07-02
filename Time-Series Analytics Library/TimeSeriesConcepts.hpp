#pragma once

#include <concepts>
#include <type_traits>

namespace tseries::concepts {

// Accepts native floating point and custom fixed-point wrappers that can convert to double.
template<typename T>
concept FloatingPointLike = std::same_as<T, float> || std::same_as<T, double>;

template<typename T>
concept FixedPointLike = !FloatingPointLike<T> && requires(T value) {
    { static_cast<double>(value) } -> std::convertible_to<double>;
};

template<typename T>
concept FloatingPointOrFixed = FloatingPointLike<T> || FixedPointLike<T>;

// A minimal trait for numeric vector expressions.
template<typename T>
concept NumericValue = requires(T value) {
    { value + value } -> std::convertible_to<T>;
    { value - value } -> std::convertible_to<T>;
    { value * value } -> std::convertible_to<T>;
    { value / value } -> std::convertible_to<T>;
};

template<typename T>
concept ArithmeticCompatible = FloatingPointOrFixed<T> && NumericValue<T>;

} // namespace tseries::concepts
