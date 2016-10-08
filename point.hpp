#pragma once

#include "utils.hpp"

#include <cmath>

namespace plot
{

using Coord = std::ptrdiff_t;

struct Point;

constexpr Point operator+(Point const& lhs, Point const& rhs);
constexpr Point operator-(Point const& lhs, Point const& rhs);
constexpr Point operator*(Point const& lhs, Coord const& rhs);
constexpr Point operator*(Coord const& lhs, Point const& rhs);
constexpr Point operator/(Point const& lhs, Coord const& rhs);
constexpr Point operator/(Coord const& lhs, Point const& rhs);

struct Point {
    constexpr Point() = default;

    constexpr Point(Coord x, Coord y)
        : x(x), y(y)
        {}

    constexpr Coord distance(Point const& other) const {
        return (other - *this).abs();
    }

    constexpr Coord abs() const {
        return std::sqrt(x*x + y*y);
    }

    constexpr Point clamp(Point const& min, Point const& max) const {
        return {
            utils::clamp(x, min.x, max.x),
            utils::clamp(y, min.y, max.y)
        };
    }

    Point& operator+=(Point const& other) {
        return (*this) = (*this) + other;
    }

    Point& operator-=(Point const& other) {
        return (*this) = (*this) - other;
    }

    Point& operator*=(Coord n) {
        return (*this) = (*this) * n;
    }

    Point& operator/=(Coord n) {
        return (*this) = (*this) / n;
    }

    constexpr bool operator==(Point const& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(Point const& other) const {
        return x != other.x || y != other.y;
    }

    Coord x = 0, y = 0;
};

inline constexpr Point operator+(Point const& lhs, Point const& rhs) {
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y
    };
}

inline constexpr Point operator-(Point const& lhs, Point const& rhs) {
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y
    };
}

inline constexpr Point operator*(Point const& lhs, Coord const& rhs) {
    return {
        lhs.x*rhs,
        lhs.y*rhs
    };
}

inline constexpr Point operator*(Coord const& lhs, Point const& rhs) {
    return {
        lhs*rhs.x,
        lhs*rhs.y
    };
}

inline constexpr Point operator/(Point const& lhs, Coord const& rhs) {
    return {
        lhs.x/rhs,
        lhs.y/rhs
    };
}

inline constexpr Point operator/(Coord const& lhs, Point const& rhs) {
    return {
        lhs/rhs.x,
        lhs/rhs.y
    };
}

using Size = Point;

} /* namespace plot */
