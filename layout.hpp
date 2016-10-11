#pragma once

#include "point.hpp"

#include <iterator>
#include <type_traits>

namespace plot
{

namespace detail
{
    template<typename Block, typename Line>
    class block_iterator
    {
    public:
        using value_type = Line;
        using reference = value_type const&;
        using pointer = value_type const*;
        using difference_type = Coord;
        using iterator_category = std::forward_iterator_tag;

        block_iterator() = default;

        reference operator*() const {
            return line;
        }

        pointer operator->() const {
            return &line;
        }

        block_iterator& operator++() {
            line = line.next();
            return *this;
        }

        block_iterator operator++(int) {
            block_iterator prev = *this;
            line = line.next();
            return prev;
        }

        bool operator==(block_iterator const& other) const {
            return line.equal(other.line);
        }

        bool operator!=(block_iterator const& other) const {
            return !line.equal(other.line);
        }

    private:
        friend Block;

        block_iterator(Line line) : line(line) {}

        Line line;
    };

    template<typename Block>
    class single_line_adapter
    {
    public:
        using value_type = Block;
        using reference = value_type const&;
        using pointer = value_type const*;
        using difference_type = Coord;
        using iterator_category = std::forward_iterator_tag;

        single_line_adapter() = default;

        reference operator*() const {
            return *block;
        }

        pointer operator->() const {
            return block;
        }

        single_line_adapter& operator++() {
            end = true;
            return *this;
        }

        single_line_adapter operator++(int) {
            single_line_adapter prev = *this;
            end = true;
            return prev;
        }

        bool operator==(single_line_adapter const& other) const {
            return end == other.end && block == other.block;
        }

        bool operator!=(single_line_adapter const& other) const {
            return end != other.end || block != other.block;
        }

    private:
        template<typename, bool>
        friend struct block_traits;

        single_line_adapter(pointer block, bool end = false)
            : block(block), end(end)
            {}

        pointer block;
        bool end;
    };

    template<typename Block>
    inline single_line_adapter<Block> operator+(typename single_line_adapter<Block>::difference_type n,
                                                single_line_adapter<Block> const& it) {
        return it + n;
    }

    template<typename Block, bool = std::is_same<Size, decltype(std::declval<Block>().size())>::value>
    struct block_traits
    {
        static Size size(Block const& block) {
            return { block.size(), 1 };
        }

        static single_line_adapter<Block> begin(Block const& block) {
            return { &block };
        }

        static single_line_adapter<Block> end(Block const& block) {
            return { &block, true };
        }
    };

    template<typename Block>
    struct block_traits<Block, true>
    {
        static Size size(Block const& block) {
            return block.size();
        }

        static auto begin(Block const& block) {
            return std::begin(block);
        }

        static auto end(Block const& block) {
            return std::end(block);
        }
    };
} /* namespace detail */

} /* namespace plot */
