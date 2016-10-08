#pragma once

#include <utility>

namespace plot
{

namespace utils
{
    template<typename T>
    inline constexpr T min(T const& a, T const& b) {
        return (a <= b) ? a : b;
    }

    template<typename T>
    inline constexpr T max(T const& a, T const& b) {
        return (a >= b) ? a : b;
    }

    template<typename T>
    inline constexpr std::pair<T, T> minmax(T const& a, T const& b) {
        return (a <= b) ? std::pair<T, T>{ a, b } : std::pair<T, T>{ b, a };
    }

    template<typename T>
    inline constexpr T clamp(T const& x, T const& mn, T const& mx) {
        return min(max(x, mn), mx);
    }

    template<typename T>
    inline constexpr T sgn(T const& x) {
        return (x > T()) - (x < T());
    }

    template<typename T>
    inline constexpr T abs(T const& x) {
        return (T() > x) ? -x : x;
    }
} /* namespace utils */


} /* namespace plot */
