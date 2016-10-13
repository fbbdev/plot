#pragma once

#include "point.hpp"

namespace plot
{

template<typename T>
struct GenericRect;

template<typename T>
constexpr GenericRect<T> operator+(GenericRect<T> const& lhs, GenericPoint<T> const& rhs);

template<typename T>
constexpr GenericRect<T> operator-(GenericRect<T> const& lhs, GenericPoint<T> const& rhs);

template<typename T>
constexpr GenericRect<T> operator*(GenericRect<T> const& lhs, T const& rhs);

template<typename T>
constexpr GenericRect<T> operator*(T const& lhs, GenericRect<T> const& rhs);

template<typename T>
constexpr GenericRect<T> operator/(GenericRect<T> const& lhs, T const& rhs);

template<typename T>
constexpr GenericRect<T> operator/(T const& lhs, GenericRect<T> const& rhs);

template<typename T>
struct GenericRect
{
    constexpr GenericRect() = default;

    constexpr GenericRect(GenericPoint<T> const& p1, GenericPoint<T> const& p2)
        : p1(p1), p2(p2)
        {}

    constexpr GenericRect(GenericSize<T> const& size)
        : p1(), p2(size)
        {}

    constexpr GenericRect sorted() const {
        auto x = utils::minmax(p1.x, p2.x);
        auto y = utils::minmax(p1.y, p2.y);
        return {
            { x.first, y.first },
            { x.second, y.second }
        };
    }

    constexpr GenericRect sorted_x() const {
        return (p1.x > p2.x) ? GenericRect(p2, p1) : *this;
    }

    constexpr GenericRect sorted_y() const {
        return (p1.y > p2.y) ? GenericRect(p2, p1) : *this;
    }

    constexpr GenericPoint<T> size() const {
        return { utils::abs(p2.x - p1.x), utils::abs(p2.y - p1.y) };
    }

    // XXX: Calling on unsorted rectangles is undefined behavior
    constexpr bool contains(GenericPoint<T> p) const {
        return p.x >= p1.x && p.x < p2.x && p.y >= p1.y && p.y < p2.y;
    }

    // XXX: Calling on unsorted rectangles is undefined behavior
    constexpr bool contains(GenericRect const& r) const {
        return r.p1.x >= p1.x && r.p2.x <= p2.x && r.p1.y >= p1.y && r.p2.y <= p2.y;
    }

    constexpr GenericRect clamp(GenericRect const& r) const {
        return { p1.clamp(r.p1, r.p2), p2.clamp(r.p1, r.p2) };
    }

    template<typename U>
    constexpr operator GenericRect<U>() const {
        return {
            static_cast<GenericPoint<U>>(p1),
            static_cast<GenericPoint<U>>(p2)
        };
    }

    GenericRect& operator+=(GenericPoint<T> const& other) {
        return (*this) = (*this) + other;
    }

    GenericRect& operator-=(GenericPoint<T> const& other) {
        return (*this) = (*this) - other;
    }

    GenericRect& operator*=(T n) {
        return (*this) = (*this) * n;
    }

    GenericRect& operator/=(T n) {
        return (*this) = (*this) / n;
    }

    constexpr bool operator==(GenericRect const& other) const {
        return p1 == other.p1 && p2 == other.p2;
    }

    constexpr bool operator!=(GenericRect const& other) const {
        return p1 != other.p1 || p2 != other.p2;
    }

    GenericPoint<T> p1{}, p2{};
};

template<typename T>
inline constexpr GenericRect<T> operator+(GenericRect<T> const& lhs, GenericPoint<T> const& rhs) {
    return {
        lhs.p1 + rhs,
        lhs.p2 + rhs
    };
}

template<typename T>
inline constexpr GenericRect<T> operator-(GenericRect<T> const& lhs, GenericPoint<T> const& rhs) {
    return {
        lhs.p1 - rhs,
        lhs.p2 - rhs
    };
}

template<typename T>
inline constexpr GenericRect<T> operator*(GenericRect<T> const& lhs, T const& rhs) {
    return {
        lhs.p1*rhs,
        lhs.p2*rhs
    };
}

template<typename T>
inline constexpr GenericRect<T> operator*(T const& lhs, GenericRect<T> const& rhs) {
    return {
        lhs*rhs.p1,
        lhs*rhs.p2
    };
}

template<typename T>
inline constexpr GenericRect<T> operator/(GenericRect<T> const& lhs, T const& rhs) {
    return {
        lhs.p1/rhs,
        lhs.p2/rhs
    };
}

template<typename T>
inline constexpr GenericRect<T> operator/(T const& lhs, GenericRect<T> const& rhs) {
    return {
        lhs/rhs.p1,
        lhs/rhs.p2
    };
}

using Rect = GenericRect<Coord>;
using Rectf = GenericRect<Coordf>;

} /* namespace plot */
