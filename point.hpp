#pragma once

#include "utils.hpp"

#include <cmath>

namespace plot
{

template<typename T>
struct GenericPoint;

template<typename T>
constexpr GenericPoint<T> operator+(GenericPoint<T> const& lhs, GenericPoint<T> const& rhs);

template<typename T>
constexpr GenericPoint<T> operator-(GenericPoint<T> const& lhs, GenericPoint<T> const& rhs);

template<typename T>
constexpr GenericPoint<T> operator*(GenericPoint<T> const& lhs, T const& rhs);

template<typename T>
constexpr GenericPoint<T> operator*(T const& lhs, GenericPoint<T> const& rhs);

template<typename T>
constexpr GenericPoint<T> operator/(GenericPoint<T> const& lhs, T const& rhs);

template<typename T>
constexpr GenericPoint<T> operator/(T const& lhs, GenericPoint<T> const& rhs);

template<typename T>
struct GenericPoint {
    constexpr GenericPoint() = default;

    constexpr GenericPoint(T x, T y)
        : x(x), y(y)
        {}

    constexpr T distance(GenericPoint const& other) const {
        return (other - *this).abs();
    }

    constexpr T abs() const {
        return std::sqrt(x*x + y*y);
    }

    constexpr GenericPoint clamp(GenericPoint const& min, GenericPoint const& max) const {
        return {
            utils::clamp(x, min.x, max.x),
            utils::clamp(y, min.y, max.y)
        };
    }

    GenericPoint& operator+=(GenericPoint const& other) {
        return (*this) = (*this) + other;
    }

    GenericPoint& operator-=(GenericPoint const& other) {
        return (*this) = (*this) - other;
    }

    GenericPoint& operator*=(T n) {
        return (*this) = (*this) * n;
    }

    GenericPoint& operator/=(T n) {
        return (*this) = (*this) / n;
    }

    constexpr bool operator==(GenericPoint const& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(GenericPoint const& other) const {
        return x != other.x || y != other.y;
    }

    template<typename U>
    constexpr operator GenericPoint<U>() const {
        return { static_cast<U>(x), static_cast<U>(y) };
    }

    T x = 0, y = 0;
};

template<typename T>
inline constexpr GenericPoint<T> operator+(GenericPoint<T> const& lhs, GenericPoint<T> const& rhs) {
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y
    };
}

template<typename T>
inline constexpr GenericPoint<T> operator-(GenericPoint<T> const& lhs, GenericPoint<T> const& rhs) {
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y
    };
}

template<typename T>
inline constexpr GenericPoint<T> operator*(GenericPoint<T> const& lhs, T const& rhs) {
    return {
        lhs.x*rhs,
        lhs.y*rhs
    };
}

template<typename T>
inline constexpr GenericPoint<T> operator*(T const& lhs, GenericPoint<T> const& rhs) {
    return {
        lhs*rhs.x,
        lhs*rhs.y
    };
}

template<typename T>
inline constexpr GenericPoint<T> operator/(GenericPoint<T> const& lhs, T const& rhs) {
    return {
        lhs.x/rhs,
        lhs.y/rhs
    };
}

template<typename T>
inline constexpr GenericPoint<T> operator/(T const& lhs, GenericPoint<T> const& rhs) {
    return {
        lhs/rhs.x,
        lhs/rhs.y
    };
}

using Coord = std::ptrdiff_t;
using Coordf = float;

using Point = GenericPoint<Coord>;
using Pointf = GenericPoint<Coordf>;

template<typename T>
using GenericSize = GenericPoint<T>;

using Size = GenericSize<Coord>;
using Sizef = GenericSize<Coordf>;

} /* namespace plot */
