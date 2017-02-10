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

namespace plot
{

namespace detail
{
    // Code points in classes Cc, Cf, Cn, Cs, Me, Mn, Zl, Zp,
    // and in range U+1160..U+11FF (Korean combining characters)
    // are given zero width.
    //
    // Code points with East_Asian_Width property set to F (Fullwidth)
    // or W (Wide) are given double width.
    //
    // For all other code points, return 1.
    std::size_t wcwidth(char32_t cp) {
        if (unicode_cp_in_tree(cp, unicode_zero_width<>))
            return 0;

        if (unicode_cp_in_tree(cp, unicode_double_width<>))
            return 2;

        return 1;
    }
} /* namespace detail */

} /* namespace plot */
