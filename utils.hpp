#pragma once

#include <utility>

namespace plot
{

namespace utils
{
    template<typename T>
    inline constexpr T min(T a, T b) {
        return (a <= b) ? a : b;
    }

    template<typename T>
    inline constexpr T max(T a, T b) {
        return (a >= b) ? a : b;
    }

    template<typename T>
    inline constexpr std::pair<T, T> minmax(T a, T b) {
        return (a <= b) ? std::pair<T, T>{ a, b } : std::pair<T, T>{ b, a };
    }

    template<typename T>
    inline constexpr T clamp(T x, T mn, T mx) {
        return min(max(x, mn), mx);
    }

    template<typename T>
    inline constexpr T sgn(T x) {
        return (x > T()) - (x < T());
    }

    template<typename T>
    inline constexpr T abs(T x) {
        return (T() > x) ? -x : x;
    }

    template<typename T>
    inline constexpr T gcd(T p, T q) {
        return q ? gcd(q, p % q) : abs(p);
    }
} /* namespace utils */


} /* namespace plot */
