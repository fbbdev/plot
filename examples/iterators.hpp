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

#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>

template<typename T>
class fake_ptr {
public:
    fake_ptr(T v) : v_(v) {}

    T operator*() const {
        return v_;
    }

    T const* operator->() const {
        return &v_;
    }
private:
    T v_;
};

// Simple type-agnostic range iterator. Range defined as [start,end) with step
template<typename T>
class range_iterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = fake_ptr<value_type>;
    using reference = value_type;
    using iterator_category = std::bidirectional_iterator_tag;

    range_iterator() = default;

    explicit range_iterator(T start, T end, T step)
        : start_(start), end_(end), step_(step), is_end(at_end())
        {}

    range_iterator(range_iterator const&) = default;
    range_iterator(range_iterator&&) = default;

    reference operator*() const {
        return start_;
    }

    pointer operator->() const {
        return { start_ };
    }

    range_iterator& operator++() {
        start_ += step_;
        is_end = at_end();
        return *this;
    }

    range_iterator operator++(int) {
        range_iterator prev = *this;
        start_ += step_;
        is_end = at_end();
        return prev;
    }

    range_iterator& operator--() {
        start_ -= step_;
        is_end = at_end();
        return *this;
    }

    range_iterator operator--(int) {
        range_iterator prev = *this;
        start_ -= step_;
        is_end = at_end();
        return prev;
    }

    bool operator==(range_iterator const& other) const {
        return (is_end && other.is_end) || (start_ == other.start_ && end_ == other.end_ && step_ == other.step_);
    }

    bool operator!=(range_iterator const& other) const {
        return !(is_end && other.is_end) && (start_ != other.start_ || end_ != other.end_ || step_ != other.step_);
    }

private:
    bool at_end() const {
        return (step_ == 0) || ((step_ > 0) ? start_ >= end_ : start_ <= end_);
    }

    T start_ = {}, end_ = {}, step_ = {};
    bool is_end = true;
};

// Simple zip iterator: given a number of iterators, for each iteration
// advance all of them and return std::make_tuple(*it...)
//
// all iterators must satisfy at least ForwardIterator
template<typename... It>
class zip_iterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<typename std::iterator_traits<It>::value_type...>;
    using pointer = fake_ptr<value_type>;
    using reference = value_type;
    using iterator_category = std::forward_iterator_tag;

    zip_iterator() = default;

    template<typename... I>
    explicit zip_iterator(I&&... iterators)
        : iterators_(std::forward<I>(iterators)...)
        {}

    zip_iterator(zip_iterator const&) = default;
    zip_iterator(zip_iterator&&) = default;

    reference operator*() const {
        return value(std::make_index_sequence<sizeof...(It)>());
    }

    pointer operator->() const {
        return pointer(value(std::make_index_sequence<sizeof...(It)>()));
    }

    zip_iterator& operator++() {
        advance(std::make_index_sequence<sizeof...(It)>());
        return *this;
    }

    zip_iterator operator++(int) {
        zip_iterator prev = *this;
        advance(std::make_index_sequence<sizeof...(It)>());
        return prev;
    }

    bool operator==(zip_iterator const& other) const {
        return iterators_ == other.iterators_;
    }

    bool operator!=(zip_iterator const& other) const {
        return iterators_ != other.iterators_;
    }

private:
    std::tuple<It...> iterators_;

    template<std::size_t... N>
    value_type value(std::index_sequence<N...>) const {
        return value_type{*std::get<N>(iterators_)...};
    }

    template<std::size_t... N>
    void advance(std::index_sequence<N...>) {
        iterators_ = std::tuple<It...>{std::next(std::get<N>(iterators_))...};
    }
};

template<typename... I>
zip_iterator<std::decay_t<I>...> zip(I&&... iterators) {
    return zip_iterator<std::decay_t<I>...>(std::forward<I>(iterators)...);
}

// Simple map iterator: given an iterator it and a function fn,
// for each iteration advance it and return fn(*it)
//
// it must satisfy at least ForwardIterator
template<typename It, typename Fn>
class map_iterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::result_of_t<Fn(typename std::iterator_traits<It>::value_type)>;
    using pointer = fake_ptr<value_type>;
    using reference = value_type;
    using iterator_category = std::forward_iterator_tag;

    map_iterator() = default;

    template<typename I, typename F>
    explicit map_iterator(I&& it, F&& fn)
        : it_(std::forward<I>(it)), fn_(std::forward<F>(fn))
        {}

    map_iterator(map_iterator const&) = default;
    map_iterator(map_iterator&&) = default;

    reference operator*() const {
        return fn_(*it_);
    }

    pointer operator->() const {
        return { fn_(*it_) };
    }

    map_iterator& operator++() {
        std::advance(it_, 1);
        return *this;
    }

    map_iterator operator++(int) {
        map_iterator prev = *this;
        std::advance(it_, 1);
        return prev;
    }

    bool operator==(map_iterator const& other) const {
        return it_ == other.it_;
    }

    bool operator!=(map_iterator const& other) const {
        return it_ != other.it_;
    }

private:
    It it_;
    Fn fn_;
};

template<typename I, typename F>
map_iterator<std::decay_t<I>, std::decay_t<F>> map(I&& it, F&& fn) {
    return map_iterator<std::decay_t<I>, std::decay_t<F>>(std::forward<I>(it), std::forward<F>(fn));
}
