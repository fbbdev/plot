/**
 * The MIT License
 *
 * Copyright (c) 2016 Fabio Massaioli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
struct GenericPoint
{
    using coord_type = T;

    constexpr GenericPoint() = default;

    constexpr GenericPoint(T xx, T yy)
        : x(xx), y(yy)
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

    template<typename U>
    constexpr operator GenericPoint<U>() const {
        return { static_cast<U>(x), static_cast<U>(y) };
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
