#pragma once

#include "FinancialVector.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace tseries {

using namespace concepts;

template<ArithmeticCompatible T>
struct RegressionResult {
    T alpha{};
    T beta{};
    T r_squared{};
};

template<ArithmeticCompatible T, VectorExpr Expr>
requires std::same_as<typename Expr::value_type, T>
[[nodiscard]] Vector<T> rolling_ema(Expr const& data, size_t period) noexcept {
    const size_t n = data.size();
    if (period == 0 || n == 0) {
        return Vector<T>{};
    }

    Vector<T> result(n);
    const T alpha = static_cast<T>(2) / (static_cast<T>(period) + static_cast<T>(1));
    result[0] = data[0];

    for (size_t i = 1; i < n; ++i) {
        const T value = data[i];
        result[i] = alpha * (value - result[i - 1]) + result[i - 1];
    }

    return result;
}

template<ArithmeticCompatible T, VectorExpr Expr>
requires std::same_as<typename Expr::value_type, T>
[[nodiscard]] Vector<T> rolling_volatility(Expr const& data, size_t period) noexcept {
    const size_t n = data.size();
    if (period < 2 || n < period) {
        return Vector<T>{};
    }

    Vector<T> result(n, static_cast<T>(0));
    const auto accumulate_cast = [&](T value) noexcept -> long double {
        return static_cast<long double>(value);
    };

    long double mean = 0.0L;
    long double m2 = 0.0L;
    size_t window_count = 0;

    for (size_t i = 0; i < n; ++i) {
        const long double x = accumulate_cast(data[i]);
        if (window_count < period) {
            ++window_count;
            const long double delta = x - mean;
            mean += delta / static_cast<long double>(window_count);
            const long double delta2 = x - mean;
            m2 += delta * delta2;
        } else {
            const long double old_value = accumulate_cast(data[i - period]);
            const long double delta_remove = old_value - mean;
            const long double nmean = mean - delta_remove / static_cast<long double>(period);
            const long double m2_remove = m2 - delta_remove * (old_value - nmean);
            mean = nmean;
            m2 = m2_remove;

            const long double delta_add = x - mean;
            mean += delta_add / static_cast<long double>(period);
            const long double delta2_add = x - mean;
            m2 += delta_add * delta2_add;
        }

        if (i + 1 >= period) {
            const long double variance = m2 / static_cast<long double>(period - 1);
            result[i] = static_cast<T>(std::sqrt(variance));
        }
    }

    return result;
}

template<ArithmeticCompatible T>
[[nodiscard]] RegressionResult<T> linear_regression(Vector<T> const& X, Vector<T> const& Y) noexcept {
    const size_t n = X.size();
    if (n == 0 || n != Y.size()) {
        return RegressionResult<T>{};
    }

    long double sum_x = 0.0L;
    long double sum_y = 0.0L;
    long double sum_xy = 0.0L;
    long double sum_xx = 0.0L;
    long double sum_yy = 0.0L;

    const size_t size = n;
    for (size_t i = 0; i < size; ++i) {
        const long double x = static_cast<long double>(X[i]);
        const long double y = static_cast<long double>(Y[i]);
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
        sum_yy += y * y;
    }

    const long double denominator = static_cast<long double>(size) * sum_xx - sum_x * sum_x;
    if (denominator == 0.0L) {
        return RegressionResult<T>{};
    }

    const long double beta = (static_cast<long double>(size) * sum_xy - sum_x * sum_y) / denominator;
    const long double alpha = (sum_y - beta * sum_x) / static_cast<long double>(size);

    const long double r_num = (static_cast<long double>(size) * sum_xy - sum_x * sum_y);
    const long double r_den = std::sqrt((static_cast<long double>(size) * sum_xx - sum_x * sum_x) * (static_cast<long double>(size) * sum_yy - sum_y * sum_y));
    const long double r_squared = (r_den != 0.0L) ? (r_num / r_den) * (r_num / r_den) : 0.0L;

    return RegressionResult<T>{static_cast<T>(alpha), static_cast<T>(beta), static_cast<T>(r_squared)};
}

} // namespace tseries
