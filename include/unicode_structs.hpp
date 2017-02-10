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

#include <utility>

namespace plot
{

namespace detail
{
    // Interval tree. Overlapping intervals are joined into a single one.
    struct unicode_interval_tree_t {
        char32_t center;
        std::pair<char32_t, char32_t> interval;
        unicode_interval_tree_t const* left;
        unicode_interval_tree_t const* right;
    };

    template<typename = void>
    bool unicode_cp_in_tree(char32_t cp, unicode_interval_tree_t const* tree) {
        while (tree) {
            if (cp >= tree->interval.first && cp <= tree->interval.second)
                return true;

            tree = (cp < tree->center) ? tree->left : tree->right;
        }

        return false;
    }
} /* namespace detail */

} /* namespace plot */
