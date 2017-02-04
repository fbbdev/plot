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

#include "point.hpp"
#include "string_view.hpp"
#include "terminal.hpp"
#include "utils.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <tuple>
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

        block_iterator(const Line &line) : line(line) {}

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
        friend struct normal_block_ref_traits;

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

    template<typename T>
    struct is_canvas
    {
        template<typename S>
        static constexpr bool test(decltype(std::declval<S>().char_size()) const*) {
            return true;
        }

        template<typename S>
        static constexpr bool test(...) {
            return false;
        }

        static constexpr bool value = test<T>(0);
    };

    template<typename Block, bool = std::is_same<Size, decltype(std::declval<Block>().size())>::value>
    struct normal_block_ref_traits
    {
        using iterator = single_line_adapter<Block>;

        static Size size(Block const& block) {
            return { block.size(), 1 };
        }

        static iterator begin(Block const& block) {
            return { &block };
        }

        static iterator end(Block const& block) {
            return { &block, true };
        }
    };

    template<typename Block>
    struct normal_block_ref_traits<Block, true>
    {
        using iterator = typename Block::const_iterator;

        static Size size(Block const& block) {
            return block.size();
        }

        static iterator begin(Block const& block) {
            return std::begin(block);
        }

        static iterator end(Block const& block) {
            return std::end(block);
        }
    };

    template<typename Block, bool = is_canvas<Block>::value>
    struct block_ref_traits : normal_block_ref_traits<Block> {};

    template<typename Block>
    struct block_ref_traits<Block, true> : normal_block_ref_traits<Block>
    {
        static Size size(Block const& block) {
            return block.char_size();
        }
    };

    template<typename Block>
    struct block_traits : block_ref_traits<Block> {};

    template<typename Block>
    struct block_traits<Block*>
    {
        using iterator = typename block_ref_traits<Block>::iterator;

        static auto size(Block* block) {
            return block_ref_traits<Block>::size(*block);
        }

        static auto begin(Block* block) {
            return block_ref_traits<Block>::begin(*block);
        }

        static auto end(Block* block) {
            return block_ref_traits<Block>::end(*block);
        }
    };
} /* namespace detail */


enum class Align
{
    Left,
    Center,
    Right
};

enum class VAlign
{
    Top,
    Middle,
    Bottom
};

enum class BorderStyle
{
    None,
    Solid,
    SolidBold,
    Dashed,
    DashedBold,
    Dotted,
    DottedBold,
    Double
};

struct Border {
    Border(BorderStyle style = BorderStyle::None, bool rounded_corners = false) {
        switch (style) {
            case BorderStyle::None:
                top_left = top = top_right = left = right =
                    bottom_left = bottom = bottom_right = " ";
                return;
            case BorderStyle::Solid:
                top = bottom = "─";
                left = right = "│";
                break;
            case BorderStyle::SolidBold:
                top = bottom = "━";
                left = right = "┃";
                break;
            case BorderStyle::Dashed:
                top = "╴"; bottom = "╶";
                left = "╷"; right = "╵";
                break;
            case BorderStyle::DashedBold:
                top = "╸"; bottom = "╺";
                left = "╻"; right = "╹";
                break;
            case BorderStyle::Dotted:
                top = bottom = "┈";
                left = right = "┊";
                break;
            case BorderStyle::DottedBold:
                top = bottom = "┉";
                left = right = "┋";
                break;
            case BorderStyle::Double:
                top = bottom = "═";
                left = right = "║";
                top_left = "╔"; top_right = "╗";
                bottom_left = "╚"; bottom_right = "╝";
                return;
        }

        switch (style) {
            case BorderStyle::Solid:
            case BorderStyle::Dashed:
            case BorderStyle::Dotted:
                top_left = (rounded_corners) ? "╭" : "┌";
                top_right = (rounded_corners) ? "╮" : "┐";
                bottom_left = (rounded_corners) ? "╰" : "└";
                bottom_right = (rounded_corners) ? "╯" : "┘";
                return;
            case BorderStyle::SolidBold:
            case BorderStyle::DashedBold:
            case BorderStyle::DottedBold:
                top_left = "┏"; top_right = "┓";
                bottom_left = "┗"; bottom_right = "┛";
                return;
            default:
                return;
        }
    }

    string_view top_left, top, top_right,
                left, right,
                bottom_left, bottom, bottom_right;
};


template<typename Block>
class Margin;

namespace detail
{
    template<typename Block>
    class margin_line;

    template<typename Block>
    std::ostream& operator<<(std::ostream&, margin_line<Block> const&);

    template<typename Block>
    class margin_line {
        using block_iterator = typename detail::block_traits<Block>::iterator;

        friend class detail::block_iterator<Margin<Block>, margin_line>;
        friend class Margin<Block>;

        friend std::ostream& operator<< <Block>(std::ostream&, margin_line const&);

        margin_line(Margin<Block> const* margin, std::ptrdiff_t overflow,
                    block_iterator line, block_iterator end)
            : margin(margin), overflow(overflow), line(std::move(line)), end(std::move(end))
            {}

        margin_line next() const {
            return (overflow || line == end) ? margin_line(margin, overflow + 1, line, end)
                                             : margin_line(margin, overflow, std::next(line), end);
        }

        bool equal(margin_line const& other) const {
            return line == other.line && overflow == other.overflow;
        }

        Margin<Block> const* margin = nullptr;
        std::ptrdiff_t overflow = 0;
        block_iterator line{}, end{};

    public:
        margin_line() = default;
    };

    template<typename Block>
    inline std::ostream& operator<<(std::ostream& stream, margin_line<Block> const& line) {
        auto fill = stream.fill();
        stream << std::setfill(' ');
        if (!line.overflow && line.line != line.end) {
            stream << std::setw(line.margin->left)
                   << ""
                   << *line.line
                   << std::setw(line.margin->right)
                   << "";
        } else {
            stream << std::setw(line.margin->size().x)
                   << "";
        }

        return stream << std::setfill(fill);
    }
} /* namespace detail */

template<typename Block>
class Margin {
public:
    using value_type = detail::margin_line<Block>;
    using reference = value_type const&;
    using const_reference = value_type const&;
    using const_iterator = detail::block_iterator<Margin<Block>, value_type>;
    using iterator = const_iterator;
    using difference_type = typename const_iterator::difference_type;
    using size_type = Size;

    explicit Margin(Block block)
        : block(std::move(block))
        {}

    explicit Margin(std::size_t margin, Block block)
        : top(margin), right(margin), bottom(margin), left(margin), block(std::move(block))
        {}

    explicit Margin(std::size_t v, std::size_t h, Block block)
        : top(v), right(h), bottom(v), left(h), block(std::move(block))
        {}

    explicit Margin(std::size_t top, std::size_t right, std::size_t bottom,
                    std::size_t left, Block block)
        : top(top), right(right), bottom(bottom), left(left), block(std::move(block))
        {}

    Size size() const {
        return detail::block_traits<Block>::size(block) + Size(left + right, top + bottom);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, -std::ptrdiff_t(top), detail::block_traits<Block>::begin(block), detail::block_traits<Block>::end(block) } };
    }

    const_iterator cend() const {
        return { { this, std::ptrdiff_t(bottom), detail::block_traits<Block>::end(block), detail::block_traits<Block>::end(block) } };
    }

private:
    friend std::ostream& detail::operator<< <Block>(std::ostream&, value_type const&);

    std::size_t top = 1, right = 2, bottom = 1, left = 2;
    Block block;
};

template<typename Block>
inline std::ostream& operator<<(std::ostream& stream, Margin<Block> const& margin) {
    for (auto const& line: margin)
        stream << line << '\n';

    return stream;
}

template<typename Block>
inline Margin<std::decay_t<Block>> margin(Block&& block) {
    return Margin<std::decay_t<Block>>(std::forward<Block>(block));
}

template<typename Block>
inline Margin<std::decay_t<Block>> margin(std::size_t margin, Block&& block) {
    return Margin<std::decay_t<Block>>(margin, std::forward<Block>(block));
}

template<typename Block>
inline Margin<std::decay_t<Block>> margin(std::size_t v, std::size_t h, Block&& block) {
    return Margin<std::decay_t<Block>>(v, h, std::forward<Block>(block));
}

template<typename Block>
inline Margin<std::decay_t<Block>> margin(std::size_t top, std::size_t right, std::size_t bottom,
                                          std::size_t left, Block&& block) {
    return Margin<std::decay_t<Block>>(top, right, bottom, left, std::forward<Block>(block));
}


template<typename Block>
class Frame;

namespace detail
{
    template<typename Block>
    class frame_line;

    template<typename Block>
    std::ostream& operator<<(std::ostream&, frame_line<Block> const&);

    template<typename Block>
    class frame_line {
        using block_iterator = typename detail::block_traits<Block>::iterator;

        friend class detail::block_iterator<Frame<Block>, frame_line>;
        friend class Frame<Block>;

        friend std::ostream& operator<< <Block>(std::ostream&, frame_line const&);

        frame_line(Frame<Block> const* frame, std::ptrdiff_t overflow,
                   block_iterator line, block_iterator end)
            : frame(frame), overflow(overflow), line(std::move(line)), end(std::move(end))
            {}

        frame_line next() const {
            return (overflow || line == end) ? frame_line(frame, overflow + 1, line, end)
                                             : frame_line(frame, overflow, std::next(line), end);
        }

        bool equal(frame_line const& other) const {
            return line == other.line && overflow == other.overflow;
        }

        Frame<Block> const* frame = nullptr;
        std::ptrdiff_t overflow = 0;
        block_iterator line{}, end{};

    public:
        frame_line() = default;
    };
} /* namespace detail */

template<typename Block>
class Frame {
public:
    using value_type = detail::frame_line<Block>;
    using reference = value_type const&;
    using const_reference = value_type const&;
    using const_iterator = detail::block_iterator<Frame<Block>, value_type>;
    using iterator = const_iterator;
    using difference_type = typename const_iterator::difference_type;
    using size_type = Size;

    explicit Frame(Block block, TerminalInfo term = TerminalInfo())
        : block(std::move(block)), term(term)
        {}

    explicit Frame(Border border, Block block, TerminalInfo term = TerminalInfo())
        : border(border), block(std::move(block)), term(term)
        {}

    explicit Frame(string_view label, Block block, TerminalInfo term = TerminalInfo())
        : label(label), block(std::move(block)), term(term)
        {}

    explicit Frame(string_view label, Align align, Block block, TerminalInfo term = TerminalInfo())
        : label(label), align(align), block(std::move(block)), term(term)
        {}

    explicit Frame(string_view label, Border border, Block block, TerminalInfo term = TerminalInfo())
        : label(label), border(border), block(std::move(block)), term(term)
        {}

    explicit Frame(string_view label, Align align, Border border, Block block, TerminalInfo term = TerminalInfo())
        : label(label), align(align), border(border), block(std::move(block)), term(term)
        {}

    Size size() const {
        return detail::block_traits<Block>::size(block) + Size(2, 2);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, -1, detail::block_traits<Block>::begin(block), detail::block_traits<Block>::end(block) } };
    }

    const_iterator cend() const {
        return { { this, 1, detail::block_traits<Block>::end(block), detail::block_traits<Block>::end(block) } };
    }

private:
    friend std::ostream& detail::operator<< <Block>(std::ostream&, value_type const&);

    string_view label;
    Align align = Align::Left;
    Border border{BorderStyle::Solid};
    Block block;
    TerminalInfo term;
};

template<typename Block>
inline std::ostream& operator<<(std::ostream& stream, Frame<Block> const& frame) {
    for (auto const& line: frame)
        stream << line << '\n';

    return stream;
}

namespace detail
{
    template<typename Block>
    std::ostream& operator<<(std::ostream& stream, frame_line<Block> const& line) {
        auto size = detail::block_traits<Block>::size(line.frame->block);
        auto label_margin = std::size_t(size.x) - utils::min(std::size_t(size.x), line.frame->label.size());
        auto const border = line.frame->border;

        if (line.overflow < 0) {
            std::size_t before_label =
                (line.frame->align == Align::Center) ? label_margin / 2
                                                     : (line.frame->align == Align::Right) ? label_margin : 0;
            std::size_t after_label = label_margin - before_label;

            stream << line.frame->term.reset() << border.top_left;

            for (; before_label > 0; --before_label)
                stream << border.top;

            stream << line.frame->label.substr(0, size.x);

            for (; after_label > 0; --after_label)
                stream << border.top;

            stream << border.top_right;

            return stream;
        } else if (line.line == line.end) {
            stream << line.frame->term.reset() << border.bottom_left;

            for (auto i = 0; i < size.x; ++i)
                stream << border.bottom;

            return stream << border.bottom_right;
        }

        return stream << line.frame->term.reset()
                      << line.frame->border.left
                      << *line.line
                      << line.frame->term.reset()
                      << line.frame->border.right;
    }
} /* namespace detail */

template<typename Block>
inline Frame<std::decay_t<Block>> frame(Block&& block, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(std::forward<Block>(block), term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(Border border, Block&& block, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(border, std::forward<Block>(block), term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(string_view label, Block&& block, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(label, std::forward<Block>(block), term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(string_view label, Align align, Block&& block, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(label, align, std::forward<Block>(block), term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(string_view label, Border border, Block&& block, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(label, border, std::forward<Block>(block), term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(string_view label, Align align, Border border, Block&& block, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(label, align, border, std::forward<Block>(block), term);
}


template<typename... Blocks>
class VBox;

namespace detail
{
    struct plus_identity {};

    template<typename Arg>
    inline constexpr decltype(auto) operator+(plus_identity, Arg&& arg) {
        return std::forward<Arg>(arg);
    }

    template<typename Arg>
    inline constexpr decltype(auto) operator+(Arg&& arg, plus_identity) {
        return std::forward<Arg>(arg);
    }

    inline constexpr plus_identity plus_fold() {
        return {};
    }

    template<typename Arg, typename... Args>
    inline constexpr auto plus_fold(Arg&& first, Args&&... rest) {
        return std::forward<Arg>(first) + plus_fold(std::forward<Args>(rest)...);
    }

    inline constexpr std::size_t find_true(std::size_t index) {
        return index;
    }

    template<typename... Args>
    inline constexpr std::size_t find_true(std::size_t index, bool first, Args&&... rest) {
        return first ? index : find_true(index + 1, std::forward<Args>(rest)...);
    }

    template<typename... Args>
    inline constexpr std::size_t find_true(bool first, Args&&... rest) {
        return first ? 0 : find_true(std::size_t(1), std::forward<Args>(rest)...);
    }

    template<typename... Blocks>
    class vbox_line;

    template<typename... Blocks>
    std::ostream& operator<<(std::ostream&, vbox_line<Blocks...> const&);

    template<typename... Blocks>
    class vbox_line
    {
        using block_iterators = std::tuple<typename detail::block_traits<Blocks>::iterator...>;

        friend class detail::block_iterator<VBox<Blocks...>, vbox_line>;
        friend class VBox<Blocks...>;

        friend std::ostream& operator<< <Blocks...>(std::ostream&, vbox_line const&);

        vbox_line(VBox<Blocks...> const* vbox, std::size_t margin,
                  block_iterators lines, block_iterators ends)
            : vbox(vbox), margin(margin), lines(std::move(lines)), ends(std::move(ends))
            {}

        vbox_line next() const {
            return margin ? vbox_line(vbox, margin - 1, lines, ends)
                          : next_impl(std::make_index_sequence<sizeof...(Blocks)>());
        }

        bool equal(vbox_line const& other) const {
            return lines == other.lines && margin == other.margin;
        }

        template<std::size_t... N>
        vbox_line next_impl(std::index_sequence<N...> indices) const {
            auto current = current_index(indices);
            block_iterators next(((N != current) ? std::get<N>(lines) : std::next(std::get<N>(lines)))...);
            return {
                vbox,
                (find_true((std::get<N>(next) != std::get<N>(ends))...) != current) ? vbox->margin : 0,
                next,
                ends
            };
        }

        template<std::size_t... N>
        std::size_t current_index(std::index_sequence<N...>) const {
            return find_true((std::get<N>(lines) != std::get<N>(ends))...);
        }

        VBox<Blocks...> const* vbox;
        std::size_t margin = 0;
        block_iterators lines, ends;

    public:
        vbox_line() = default;
    };

    inline std::ostream& output_vbox_line(std::ostream& stream, std::size_t) {
        return stream;
    }

    template<typename Arg, typename... Args>
    inline std::ostream& output_vbox_line(std::ostream& stream, std::size_t width, Arg&& first, Args&&... rest) {
        return (std::get<0>(std::forward<Arg>(first)) != std::get<1>(std::forward<Arg>(first)))
            ? (stream << *std::get<0>(std::forward<Arg>(first))
                      << std::setw(width - detail::block_traits<std::decay_t<std::tuple_element_t<2, std::decay_t<Arg>>>>
                          ::size(std::get<2>(std::forward<Arg>(first))).x) << "")
            : output_vbox_line(stream, width, std::forward<Args>(rest)...);
    }

    template<typename Iterators, typename Blocks, std::size_t... N>
    inline std::ostream& output_vbox_line(std::ostream& stream,
                                          std::size_t width,
                                          Iterators const& lines,
                                          Iterators const& ends,
                                          Blocks const& blocks,
                                          std::index_sequence<N...>) {
        return output_vbox_line(
            stream, width, std::forward_as_tuple(std::get<N>(lines), std::get<N>(ends), std::get<N>(blocks))...);
    }

    template<typename... Blocks>
    inline std::ostream& operator<<(std::ostream& stream, vbox_line<Blocks...> const& line) {
        auto fill = stream.fill();
        auto width = line.vbox->size().x;

        stream << std::setfill(' ');

        if (!line.margin)
            output_vbox_line(stream, width, line.lines, line.ends, line.vbox->blocks,
                             std::make_index_sequence<sizeof...(Blocks)>());
        else
            stream << std::setw(width) << "";

        return stream << std::setfill(fill);
    }
} /* namespace detail */

template<typename... Blocks>
class VBox
{
public:
    using value_type = detail::vbox_line<Blocks...>;
    using reference = value_type const&;
    using const_reference = value_type const&;
    using const_iterator = detail::block_iterator<VBox<Blocks...>, value_type>;
    using iterator = const_iterator;
    using difference_type = typename const_iterator::difference_type;
    using size_type = Size;

    explicit VBox(Blocks... blocks)
        : blocks(std::move(blocks)...)
        {}

    explicit VBox(std::size_t margin, Blocks... blocks)
        : margin(margin), blocks(std::move(blocks)...)
        {}

    Size size() const {
        return size_impl(std::make_index_sequence<sizeof...(Blocks)>()) +
            Size(0, margin*(sizeof...(Blocks) - 1));
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, 0, begins(std::make_index_sequence<sizeof...(Blocks)>()),
                   ends(std::make_index_sequence<sizeof...(Blocks)>()) } };
    }

    const_iterator cend() const {
        auto e = ends(std::make_index_sequence<sizeof...(Blocks)>());
        return { { this, margin, e, e } };
    }

private:
    friend value_type;

    friend std::ostream& detail::operator<< <Blocks...>(std::ostream&, value_type const&);

    template<std::size_t... N>
    Size size_impl(std::index_sequence<N...>) const {
        return {
            std::max({ detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                ::size(std::get<N>(blocks)).x... }),
            detail::plus_fold(
                detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                    ::size(std::get<N>(blocks)).y...)
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators begins(std::index_sequence<N...>) const {
        return {
            detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                ::begin(std::get<N>(blocks))...
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators ends(std::index_sequence<N...>) const {
        return {
            detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                ::end(std::get<N>(blocks))...
        };
    }

    std::size_t margin = 1;
    std::tuple<Blocks...> blocks;
};

template<typename Block, typename... Blocks>
inline std::enable_if_t<!std::is_convertible<std::decay_t<Block>, std::size_t>::value,
                        VBox<std::decay_t<Block>, std::decay_t<Blocks>...>>
vbox(Block&& block, Blocks&&... blocks) {
    return VBox<std::decay_t<Block>, std::decay_t<Blocks>...>(std::forward<Block>(block), std::forward<Blocks>(blocks)...);
}

template<typename Margin, typename... Blocks>
inline std::enable_if_t<std::is_convertible<std::decay_t<Margin>, std::size_t>::value,
                        VBox<std::decay_t<Blocks>...>>
vbox(Margin&& margin, Blocks&&... blocks) {
    return VBox<std::decay_t<Blocks>...>(std::forward<Margin>(margin), std::forward<Blocks>(blocks)...);
}


template<typename... Blocks>
class HBox;

namespace detail
{
    template<typename... Blocks>
    class hbox_line;

    template<typename... Blocks>
    std::ostream& operator<<(std::ostream&, hbox_line<Blocks...> const&);

    template<typename... Blocks>
    class hbox_line
    {
        using block_iterators = std::tuple<typename detail::block_traits<Blocks>::iterator...>;

        friend class detail::block_iterator<HBox<Blocks...>, hbox_line>;
        friend class HBox<Blocks...>;

        friend std::ostream& operator<< <Blocks...>(std::ostream&, hbox_line const&);

        hbox_line(HBox<Blocks...> const* hbox, std::size_t margin, block_iterators lines, block_iterators ends)
            : hbox(hbox), margin(margin), lines(std::move(lines)), ends(std::move(ends))
            {}

        hbox_line next() const {
            return next_impl(std::make_index_sequence<sizeof...(Blocks)>());
        }

        bool equal(hbox_line const& other) const {
            return lines == other.lines;
        }

        template<std::size_t... N>
        hbox_line next_impl(std::index_sequence<N...>) const {
            return {
                hbox,
                margin,
                { ((std::get<N>(lines) != std::get<N>(ends)) ? std::next(std::get<N>(lines)) : std::get<N>(lines))... },
                ends
            };
        }

        HBox<Blocks...> const* hbox;
        std::size_t margin = 0;
        block_iterators lines, ends;

    public:
        hbox_line() = default;
    };

    inline std::ostream& output_hbox_line(std::ostream& stream, std::size_t) {
        return stream;
    }

    template<typename Arg, typename... Args>
    inline std::ostream& output_hbox_line(std::ostream& stream, std::size_t margin,
                                          Arg&& first, Args&&... rest) {
        return output_hbox_line(
            ((std::get<0>(std::forward<Arg>(first)) != std::get<1>(std::forward<Arg>(first)))
                ? stream << std::setw(margin) << "" << *std::get<0>(std::forward<Arg>(first))
                : stream << std::setw(margin + detail::block_traits<std::decay_t<std::tuple_element_t<2, std::decay_t<Arg>>>>
                    ::size(std::get<2>(std::forward<Arg>(first))).x) << ""),
            margin, std::forward<Args>(rest)...);
    }

    template<typename Iterators, typename Blocks, std::size_t... N>
    inline std::ostream& output_hbox_line(std::ostream& stream,
                                          std::size_t margin,
                                          Iterators const& lines,
                                          Iterators const& ends,
                                          Blocks const& blocks,
                                          std::index_sequence<N...>) {
        return output_hbox_line(
            ((std::get<0>(lines) != std::get<0>(ends))
                ? stream << *std::get<0>(lines)
                : stream << std::setw(detail::block_traits<std::decay_t<std::tuple_element_t<0, Blocks>>>
                    ::size(std::get<0>(blocks)).x) << ""),
            margin, std::forward_as_tuple(std::get<N+1>(lines), std::get<N+1>(ends), std::get<N+1>(blocks))...);
    }

    template<typename... Blocks>
    inline std::ostream& operator<<(std::ostream& stream, hbox_line<Blocks...> const& line) {
        auto fill = stream.fill();
        return output_hbox_line(stream << std::setfill(' '), line.margin, line.lines, line.ends, line.hbox->blocks,
                                std::make_index_sequence<sizeof...(Blocks) - 1>()) << std::setfill(fill);
    }
} /* namespace detail */

template<typename... Blocks>
class HBox
{
public:
    using value_type = detail::hbox_line<Blocks...>;
    using reference = value_type const&;
    using const_reference = value_type const&;
    using const_iterator = detail::block_iterator<HBox<Blocks...>, value_type>;
    using iterator = const_iterator;
    using difference_type = typename const_iterator::difference_type;
    using size_type = Size;

    explicit HBox(Blocks... blocks)
        : blocks(std::move(blocks)...)
        {}

    explicit HBox(std::size_t margin, Blocks... blocks)
        : margin(margin), blocks(std::move(blocks)...)
        {}

    Size size() const {
        return size_impl(std::make_index_sequence<sizeof...(Blocks)>()) +
            Size(margin*(sizeof...(Blocks) - 1), 0);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, margin, begins(std::make_index_sequence<sizeof...(Blocks)>()),
                   ends(std::make_index_sequence<sizeof...(Blocks)>()) } };
    }

    const_iterator cend() const {
        auto e = ends(std::make_index_sequence<sizeof...(Blocks)>());
        return { { this, margin, e, e } };
    }

private:
    friend std::ostream& detail::operator<< <Blocks...>(std::ostream&, value_type const&);

    template<std::size_t... N>
    Size size_impl(std::index_sequence<N...>) const {
        return {
            detail::plus_fold(
                detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                    ::size(std::get<N>(blocks)).x...),
            std::max({ detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                ::size(std::get<N>(blocks)).y... }),
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators begins(std::index_sequence<N...>) const {
        return {
            detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                ::begin(std::get<N>(blocks))...
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators ends(std::index_sequence<N...>) const {
        return {
            detail::block_traits<std::tuple_element_t<N, decltype(blocks)>>
                ::end(std::get<N>(blocks))...
        };
    }

    std::size_t margin = 2;
    std::tuple<Blocks...> blocks;
};

template<typename Block, typename... Blocks>
inline std::enable_if_t<!std::is_convertible<std::decay_t<Block>, std::size_t>::value,
                        HBox<std::decay_t<Block>, std::decay_t<Blocks>...>>
hbox(Block&& block, Blocks&&... blocks) {
    return HBox<std::decay_t<Block>, std::decay_t<Blocks>...>(std::forward<Block>(block), std::forward<Blocks>(blocks)...);
}

template<typename Margin, typename... Blocks>
inline std::enable_if_t<std::is_convertible<std::decay_t<Margin>, std::size_t>::value,
                        HBox<std::decay_t<Blocks>...>>
hbox(Margin&& margin, Blocks&&... blocks) {
    return HBox<std::decay_t<Blocks>...>(std::forward<Margin>(margin), std::forward<Blocks>(blocks)...);
}

} /* namespace plot */
