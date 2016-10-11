#pragma once

#include <experimental/string_view>

namespace plot
{

using std::experimental::string_view;

namespace detail
{
    template<typename T>
    inline bool contains(string_view haystack, T&& needle) {
        return haystack.find(std::forward<T>(needle)) != string_view::npos;
    }
} /* namespace detail */

} /* namespace plot */
