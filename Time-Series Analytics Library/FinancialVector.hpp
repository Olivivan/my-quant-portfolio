#pragma once

#include "TimeSeriesConcepts.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

namespace tseries {

using namespace concepts;

template<typename Derived, typename ValueT>
struct VectorExpression {
    using value_type = ValueT;

    [[nodiscard]] constexpr size_t size() const noexcept {
        return static_cast<Derived const&>(*this).size();
    }

    [[nodiscard]] constexpr ValueT operator[](size_t idx) const noexcept {
        return static_cast<Derived const&>(*this)[idx];
    }
};

template<typename E>
concept VectorExpr = requires(E e, size_t idx) {
    { e.size() } noexcept -> std::convertible_to<size_t>;
    { e[idx] } noexcept;
};

template<typename Lhs, typename Rhs>
concept CompatibleVectorExpr = VectorExpr<Lhs> && VectorExpr<Rhs> && std::same_as<typename Lhs::value_type, typename Rhs::value_type>;

template<ArithmeticCompatible T>
struct Vector : VectorExpression<Vector<T>, T> {
    using value_type = T;

    Vector() noexcept = default;

    explicit Vector(size_t size, T initial = T{}) noexcept
        : data_(size, initial) {}

    Vector(std::initializer_list<T> init) noexcept
        : data_(init) {}

    template<VectorExpr E>
    requires std::convertible_to<typename E::value_type, T>
    explicit Vector(E const& expression) noexcept
        : data_(expression.size()) {
        assign(expression);
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return data_.size();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return data_.empty();
    }

    [[nodiscard]] constexpr T* data() noexcept {
        return data_.data();
    }

    [[nodiscard]] constexpr T const* data() const noexcept {
        return data_.data();
    }

    [[nodiscard]] constexpr T& operator[](size_t index) noexcept {
        assert(index < size());
        return data_[index];
    }

    [[nodiscard]] constexpr T operator[](size_t index) const noexcept {
        assert(index < size());
        return data_[index];
    }

    void resize(size_t count, T value = T{}) noexcept {
        data_.resize(count, value);
    }

    void reserve(size_t count) noexcept {
        data_.reserve(count);
    }

    template<VectorExpr E>
    requires std::convertible_to<typename E::value_type, T>
    Vector& operator=(E const& expression) noexcept {
        const size_t n = expression.size();
        if (size() != n) {
            data_.assign(n, T{});
        }
        assign(expression);
        return *this;
    }

    template<VectorExpr E>
    requires std::convertible_to<typename E::value_type, T>
    void assign(E const& expression) noexcept {
        assert(size() == expression.size());
        const size_t n = size();
        T* ptr = data();
        for (size_t i = 0; i < n; ++i) {
            ptr[i] = static_cast<T>(expression[i]);
        }
    }

private:
    std::vector<T> data_;
};

struct AddOp {
    template<typename T>
    [[nodiscard]] static constexpr T apply(T const& a, T const& b) noexcept {
        return a + b;
    }
};

struct SubOp {
    template<typename T>
    [[nodiscard]] static constexpr T apply(T const& a, T const& b) noexcept {
        return a - b;
    }
};

struct MulOp {
    template<typename T>
    [[nodiscard]] static constexpr T apply(T const& a, T const& b) noexcept {
        return a * b;
    }
};

template<typename Lhs, typename Rhs>
requires CompatibleVectorExpr<Lhs, Rhs>
[[nodiscard]] constexpr auto operator+(Lhs const& lhs, Rhs const& rhs) noexcept {
    struct Expr : VectorExpression<Expr, typename Lhs::value_type> {
        Expr(Lhs const& l, Rhs const& r) noexcept : lhs(l), rhs(r) {
            assert(l.size() == r.size());
        }
        [[nodiscard]] constexpr size_t size() const noexcept { return lhs.size(); }
        [[nodiscard]] constexpr typename Lhs::value_type operator[](size_t idx) const noexcept {
            return AddOp::apply(lhs[idx], rhs[idx]);
        }
        Lhs const& lhs;
        Rhs const& rhs;
    };
    return Expr(lhs, rhs);
}

template<typename Lhs, typename Rhs>
requires CompatibleVectorExpr<Lhs, Rhs>
[[nodiscard]] constexpr auto operator-(Lhs const& lhs, Rhs const& rhs) noexcept {
    struct Expr : VectorExpression<Expr, typename Lhs::value_type> {
        Expr(Lhs const& l, Rhs const& r) noexcept : lhs(l), rhs(r) {
            assert(l.size() == r.size());
        }
        [[nodiscard]] constexpr size_t size() const noexcept { return lhs.size(); }
        [[nodiscard]] constexpr typename Lhs::value_type operator[](size_t idx) const noexcept {
            return SubOp::apply(lhs[idx], rhs[idx]);
        }
        Lhs const& lhs;
        Rhs const& rhs;
    };
    return Expr(lhs, rhs);
}

template<VectorExpr E, ArithmeticCompatible T>
[[nodiscard]] constexpr auto operator*(E const& expr, T scalar) noexcept {
    struct Expr : VectorExpression<Expr, typename E::value_type> {
        Expr(E const& e, T s) noexcept : expression(e), scalar(s) {}
        [[nodiscard]] constexpr size_t size() const noexcept { return expression.size(); }
        [[nodiscard]] constexpr typename E::value_type operator[](size_t idx) const noexcept {
            return MulOp::apply(expression[idx], static_cast<typename E::value_type>(scalar));
        }
        E const& expression;
        T scalar;
    };
    return Expr(expr, scalar);
}

template<ArithmeticCompatible T, VectorExpr E>
[[nodiscard]] constexpr auto operator*(T scalar, E const& expr) noexcept {
    return expr * scalar;
}

} // namespace tseries
