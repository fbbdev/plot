/**
 * The MIT License
 *
 * Copyright (c) 2017 Fabio Massaioli
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

#include "unicode_data.hpp"
#include "string_view.hpp"

#include <cstdint>
#include <limits>

namespace plot
{

namespace detail
{
    // For code points in classes Cc, Cf, Cn, Cs, Me, Mn, Zl, Zp,
    // and in range U+1160..U+11FF (Korean combining characters),
    // return width 0.
    //
    // For code points with East_Asian_Width property set to F (Fullwidth)
    // or W (Wide), return width 2.
    //
    // For all other code points, return 1.
    inline std::size_t wcwidth(char32_t cp) {
        if (unicode_cp_in_tree(cp, unicode_tables<>::zero_width))
            return 0;

        if (unicode_cp_in_tree(cp, unicode_tables<>::double_width))
            return 2;

        return 1;
    }

    constexpr std::uint8_t utf8_start_masks[] = {
        0, 0b1111111, 0b11111, 0b1111, 0b111
    };

    constexpr std::uint8_t utf8_start_markers[] = {
        0, 0b00000000, 0b11000000, 0b11100000, 0b11110000
    };

    constexpr std::uint8_t utf8_cont_mask = 0b111111;
    constexpr std::uint8_t utf8_cont_marker = 0b10000000;

    inline constexpr bool utf8_seq_start(std::uint8_t byte) {
        return (byte & ~utf8_start_masks[1]) == utf8_start_markers[1] ||
               (byte & ~utf8_start_masks[2]) == utf8_start_markers[2] ||
               (byte & ~utf8_start_masks[3]) == utf8_start_markers[3] ||
               (byte & ~utf8_start_masks[4]) == utf8_start_markers[4];
    }

    inline constexpr bool utf8_seq_cont(std::uint8_t byte) {
        return (byte & ~utf8_cont_mask) == utf8_cont_marker;
    }

    inline int utf8_seq_length(std::uint8_t first) {
        if ((first & ~utf8_start_masks[1]) == utf8_start_markers[1])
            return 1;
        else if ((first & ~utf8_start_masks[2]) == utf8_start_markers[2])
            return 2;
        else if ((first & ~utf8_start_masks[3]) == utf8_start_markers[3])
            return 3;
        else if ((first & ~utf8_start_masks[4]) == utf8_start_markers[4])
            return 4;
        else
            return 0;
    }

    template<typename Iterator>
    inline Iterator utf8_next(Iterator it, Iterator end) {
        while (it != end && !utf8_seq_start(*++it)) { /* do nothing */ }
        return it;
    }

    template<typename Iterator>
    inline char32_t utf8_cp(Iterator it, Iterator end) {
        char32_t cp = static_cast<std::uint8_t>(*it);
        auto len = utf8_seq_length(cp);

        if (!len)
            return std::numeric_limits<char32_t>::max();

        cp = (cp & utf8_start_masks[len]);

        while (--len > 0 && it != end && utf8_seq_cont(*++it))
            cp = (cp << 6) | (static_cast<std::uint8_t>(*it) & utf8_cont_mask);

        while (--len > 0)
            cp <<= 6;

        return cp;
    }
} /* namespace detail */

template<typename Iterator>
std::size_t utf8_string_width(Iterator first, Iterator last) {
    std::size_t width = 0;

    for (; first != last; first = detail::utf8_next(first, last))
        width += detail::wcwidth(detail::utf8_cp(first, last));

    return width;
}

inline std::size_t utf8_string_width(string_view str) {
    return utf8_string_width(str.begin(), str.end());
}

template<typename Iterator>
std::pair<Iterator, std::size_t> utf8_clamp(Iterator first, Iterator last, std::size_t width) {
    std::size_t request = width;

    for (; first != last; first = detail::utf8_next(first, last)) {
        auto cw = detail::wcwidth(detail::utf8_cp(first, last));
        if (cw > width)
            break;
        width -= cw;
    }

    return { first, request - width };
}

inline std::pair<string_view, std::size_t> utf8_clamp(string_view str, std::size_t width) {
    auto res = utf8_clamp(str.begin(), str.end(), width);
    return { string_view(str.begin(), res.first - str.begin()), res.second };
}

} /* namespace plot */
