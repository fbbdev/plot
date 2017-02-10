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
#include "unicode.hpp"
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
            return line_;
        }

        pointer operator->() const {
            return &line_;
        }

        block_iterator& operator++() {
            line_ = line_.next();
            return *this;
        }

        block_iterator operator++(int) {
            block_iterator prev = *this;
            line_ = line_.next();
            return prev;
        }

        bool operator==(block_iterator const& other) const {
            return line_.equal(other.line_);
        }

        bool operator!=(block_iterator const& other) const {
            return !line_.equal(other.line_);
        }

    private:
        friend Block;

        block_iterator(Line line) : line_(std::move(line)) {}

        Line line_;
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
            return *block_;
        }

        pointer operator->() const {
            return block_;
        }

        single_line_adapter& operator++() {
            end_ = true;
            return *this;
        }

        single_line_adapter operator++(int) {
            single_line_adapter prev = *this;
            end_ = true;
            return prev;
        }

        bool operator==(single_line_adapter const& other) const {
            return end_ == other.end_ && block_ == other.block_;
        }

        bool operator!=(single_line_adapter const& other) const {
            return end_ != other.end_ || block_ != other.block_;
        }

    private:
        template<typename, bool>
        friend struct normal_block_ref_traits;

        single_line_adapter(pointer block, bool end = false)
            : block_(block), end_(end)
            {}

        pointer block_;
        bool end_;
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
                    bottom_left = bottom = bottom_right = u8" ";
                return;
            case BorderStyle::Solid:
                top = bottom = u8"─";
                left = right = u8"│";
                break;
            case BorderStyle::SolidBold:
                top = bottom = u8"━";
                left = right = u8"┃";
                break;
            case BorderStyle::Dashed:
                top = u8"╴"; bottom = u8"╶";
                left = u8"╷"; right = u8"╵";
                break;
            case BorderStyle::DashedBold:
                top = u8"╸"; bottom = u8"╺";
                left = u8"╻"; right = u8"╹";
                break;
            case BorderStyle::Dotted:
                top = bottom = u8"┈";
                left = right = u8"┊";
                break;
            case BorderStyle::DottedBold:
                top = bottom = u8"┉";
                left = right = u8"┋";
                break;
            case BorderStyle::Double:
                top = bottom = u8"═";
                left = right = u8"║";
                top_left = u8"╔"; top_right = u8"╗";
                bottom_left = u8"╚"; bottom_right = u8"╝";
                return;
        }

        switch (style) {
            case BorderStyle::Solid:
            case BorderStyle::Dashed:
            case BorderStyle::Dotted:
                top_left = (rounded_corners) ? u8"╭" : u8"┌";
                top_right = (rounded_corners) ? u8"╮" : u8"┐";
                bottom_left = (rounded_corners) ? u8"╰" : u8"└";
                bottom_right = (rounded_corners) ? u8"╯" : u8"┘";
                return;
            case BorderStyle::SolidBold:
            case BorderStyle::DashedBold:
            case BorderStyle::DottedBold:
                top_left = u8"┏"; top_right = u8"┓";
                bottom_left = u8"┗"; bottom_right = u8"┛";
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
            : margin_(margin), overflow_(overflow), line_(std::move(line)), end_(std::move(end))
            {}

        margin_line next() const {
            return (overflow_ || line_ == end_) ? margin_line(margin_, overflow_ + 1, line_, end_)
                                                : margin_line(margin_, overflow_, std::next(line_), end_);
        }

        bool equal(margin_line const& other) const {
            return line_ == other.line_ && overflow_ == other.overflow_;
        }

        Margin<Block> const* margin_ = nullptr;
        std::ptrdiff_t overflow_ = 0;
        block_iterator line_{}, end_{};

    public:
        margin_line() = default;
    };

    template<typename Block>
    inline std::ostream& operator<<(std::ostream& stream, margin_line<Block> const& line) {
        auto fill = stream.fill();
        stream << std::setfill(' ');
        if (!line.overflow_ && line.line_ != line.end_) {
            stream << std::setw(line.margin_->left_)
                   << u8""
                   << *line.line_
                   << std::setw(line.margin_->right_)
                   << u8"";
        } else {
            stream << std::setw(line.margin_->size().x)
                   << u8"";
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
        : block_(std::move(block))
        {}

    explicit Margin(std::size_t margin, Block block)
        : top_(margin), right_(margin), bottom_(margin), left_(margin), block_(std::move(block))
        {}

    explicit Margin(std::size_t v, std::size_t h, Block block)
        : top_(v), right_(h), bottom_(v), left_(h), block_(std::move(block))
        {}

    explicit Margin(std::size_t top, std::size_t right, std::size_t bottom,
                    std::size_t left, Block block)
        : top_(top), right_(right), bottom_(bottom), left_(left), block_(std::move(block))
        {}

    Size size() const {
        return detail::block_traits<Block>::size(block_) + Size(left_ + right_, top_ + bottom_);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, -std::ptrdiff_t(top_), detail::block_traits<Block>::begin(block_), detail::block_traits<Block>::end(block_) } };
    }

    const_iterator cend() const {
        return { { this, std::ptrdiff_t(bottom_), detail::block_traits<Block>::end(block_), detail::block_traits<Block>::end(block_) } };
    }

private:
    friend std::ostream& detail::operator<< <Block>(std::ostream&, value_type const&);

    std::size_t top_ = 1, right_ = 2, bottom_ = 1, left_ = 2;
    Block block_;
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
            : frame_(frame), overflow_(overflow), line_(std::move(line)), end_(std::move(end))
        {
            auto max_width = detail::block_traits<Block>::size(frame->block_).x;
            std::tie(label_, label_width_) = utf8_clamp(frame->label_, max_width);
        }

        frame_line(Frame<Block> const* frame, std::ptrdiff_t overflow,
                   string_view label, std::size_t label_width,
                   block_iterator line, block_iterator end)
            : frame_(frame), overflow_(overflow),
              label_(label), label_width_(label_width),
              line_(std::move(line)), end_(std::move(end))
            {}

        frame_line next() const {
            return (overflow_ || line_ == end_) ? frame_line(frame_, overflow_ + 1, label_, label_width_, line_, end_)
                                                : frame_line(frame_, overflow_, label_, label_width_, std::next(line_), end_);
        }

        bool equal(frame_line const& other) const {
            return line_ == other.line_ && overflow_ == other.overflow_;
        }

        Frame<Block> const* frame_ = nullptr;
        std::ptrdiff_t overflow_ = 0;
        string_view label_{};
        std::size_t label_width_ = 0;
        block_iterator line_{}, end_{};

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
        : block_(std::move(block)), term_(term)
        {}

    explicit Frame(Border border, Block block, TerminalInfo term = TerminalInfo())
        : border_(border), block_(std::move(block)), term_(term)
        {}

    explicit Frame(string_view label, Block block, TerminalInfo term = TerminalInfo())
        : label_(label), block_(std::move(block)), term_(term)
        {}

    explicit Frame(string_view label, Align align, Block block, TerminalInfo term = TerminalInfo())
        : label_(label), align_(align), block_(std::move(block)), term_(term)
        {}

    explicit Frame(string_view label, Border border, Block block, TerminalInfo term = TerminalInfo())
        : label_(label), border_(border), block_(std::move(block)), term_(term)
        {}

    explicit Frame(string_view label, Align align, Border border, Block block, TerminalInfo term = TerminalInfo())
        : label_(label), align_(align), border_(border), block_(std::move(block)), term_(term)
        {}

    Size size() const {
        return detail::block_traits<Block>::size(block_) + Size(2, 2);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, -1, detail::block_traits<Block>::begin(block_), detail::block_traits<Block>::end(block_) } };
    }

    const_iterator cend() const {
        return { { this, 1, detail::block_traits<Block>::end(block_), detail::block_traits<Block>::end(block_) } };
    }

private:
    template<typename>
    friend class detail::frame_line;
    friend std::ostream& detail::operator<< <Block>(std::ostream&, value_type const&);

    string_view label_;
    Align align_ = Align::Left;
    Border border_{BorderStyle::Solid};
    Block block_;
    TerminalInfo term_;
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
        auto size = detail::block_traits<Block>::size(line.frame_->block_);
        auto label_margin = std::size_t(size.x) - line.label_width_;
        auto const border = line.frame_->border_;

        if (line.overflow_ < 0) {
            std::size_t before_label =
                (line.frame_->align_ == Align::Center) ? label_margin / 2
                                                       : (line.frame_->align_ == Align::Right) ? label_margin : 0;
            std::size_t after_label = label_margin - before_label;

            stream << line.frame_->term_.reset() << border.top_left;

            for (; before_label > 0; --before_label)
                stream << border.top;

            stream << line.label_;

            for (; after_label > 0; --after_label)
                stream << border.top;

            stream << border.top_right;

            return stream;
        } else if (line.line_ == line.end_) {
            stream << line.frame_->term_.reset() << border.bottom_left;

            for (auto i = 0; i < size.x; ++i)
                stream << border.bottom;

            return stream << border.bottom_right;
        }

        return stream << line.frame_->term_.reset()
                      << line.frame_->border_.left
                      << *line.line_
                      << line.frame_->term_.reset()
                      << line.frame_->border_.right;
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
            : vbox_(vbox), margin_(margin), lines_(std::move(lines)), ends_(std::move(ends))
            {}

        vbox_line next() const {
            return margin_ ? vbox_line(vbox_, margin_ - 1, lines_, ends_)
                           : next_impl(std::make_index_sequence<sizeof...(Blocks)>());
        }

        bool equal(vbox_line const& other) const {
            return lines_ == other.lines_ && margin_ == other.margin_;
        }

        template<std::size_t... N>
        vbox_line next_impl(std::index_sequence<N...> indices) const {
            auto current = current_index(indices);
            block_iterators nxt(((N != current) ? std::get<N>(lines_) : std::next(std::get<N>(lines_)))...);
            return {
                vbox_,
                (find_true((std::get<N>(nxt) != std::get<N>(ends_))...) != current) ? vbox_->margin_ : 0,
                nxt,
                ends_
            };
        }

        template<std::size_t... N>
        std::size_t current_index(std::index_sequence<N...>) const {
            return find_true((std::get<N>(lines_) != std::get<N>(ends_))...);
        }

        VBox<Blocks...> const* vbox_;
        std::size_t margin_ = 0;
        block_iterators lines_, ends_;

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
                          ::size(std::get<2>(std::forward<Arg>(first))).x) << u8"")
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
        auto width = line.vbox_->size().x;

        stream << std::setfill(' ');

        if (!line.margin_)
            output_vbox_line(stream, width, line.lines_, line.ends_, line.vbox_->blocks_,
                             std::make_index_sequence<sizeof...(Blocks)>());
        else
            stream << std::setw(width) << u8"";

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
        : blocks_(std::move(blocks)...)
        {}

    explicit VBox(std::size_t margin, Blocks... blocks)
        : margin_(margin), blocks_(std::move(blocks)...)
        {}

    Size size() const {
        return size_impl(std::make_index_sequence<sizeof...(Blocks)>()) +
            Size(0, margin_*(sizeof...(Blocks) - 1));
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
        return { { this, margin_, e, e } };
    }

private:
    friend value_type;

    friend std::ostream& detail::operator<< <Blocks...>(std::ostream&, value_type const&);

    template<std::size_t... N>
    Size size_impl(std::index_sequence<N...>) const {
        return {
            std::max({ detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                ::size(std::get<N>(blocks_)).x... }),
            detail::plus_fold(
                detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                    ::size(std::get<N>(blocks_)).y...)
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators begins(std::index_sequence<N...>) const {
        return typename value_type::block_iterators{
            detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                ::begin(std::get<N>(blocks_))...
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators ends(std::index_sequence<N...>) const {
        return typename value_type::block_iterators{
            detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                ::end(std::get<N>(blocks_))...
        };
    }

    std::size_t margin_ = 1;
    std::tuple<Blocks...> blocks_;
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
            : hbox_(hbox), margin_(margin), lines_(std::move(lines)), ends_(std::move(ends))
            {}

        hbox_line next() const {
            return next_impl(std::make_index_sequence<sizeof...(Blocks)>());
        }

        bool equal(hbox_line const& other) const {
            return lines_ == other.lines_;
        }

        template<std::size_t... N>
        hbox_line next_impl(std::index_sequence<N...>) const {
            return {
                hbox_,
                margin_,
                block_iterators{
                    ((std::get<N>(lines_) != std::get<N>(ends_)) ? std::next(std::get<N>(lines_)) : std::get<N>(lines_))... },
                ends_
            };
        }

        HBox<Blocks...> const* hbox_;
        std::size_t margin_ = 0;
        block_iterators lines_, ends_;

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
                ? stream << std::setw(margin) << u8"" << *std::get<0>(std::forward<Arg>(first))
                : stream << std::setw(margin + detail::block_traits<std::decay_t<std::tuple_element_t<2, std::decay_t<Arg>>>>
                    ::size(std::get<2>(std::forward<Arg>(first))).x) << u8""),
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
                    ::size(std::get<0>(blocks)).x) << u8""),
            margin, std::forward_as_tuple(std::get<N+1>(lines), std::get<N+1>(ends), std::get<N+1>(blocks))...);
    }

    template<typename... Blocks>
    inline std::ostream& operator<<(std::ostream& stream, hbox_line<Blocks...> const& line) {
        auto fill = stream.fill();
        return output_hbox_line(stream << std::setfill(' '), line.margin_, line.lines_, line.ends_, line.hbox_->blocks_,
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
        : blocks_(std::move(blocks)...)
        {}

    explicit HBox(std::size_t margin, Blocks... blocks)
        : margin_(margin), blocks_(std::move(blocks)...)
        {}

    Size size() const {
        return size_impl(std::make_index_sequence<sizeof...(Blocks)>()) +
            Size(margin_*(sizeof...(Blocks) - 1), 0);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, margin_, begins(std::make_index_sequence<sizeof...(Blocks)>()),
                   ends(std::make_index_sequence<sizeof...(Blocks)>()) } };
    }

    const_iterator cend() const {
        auto e = ends(std::make_index_sequence<sizeof...(Blocks)>());
        return { { this, margin_, e, e } };
    }

private:
    friend std::ostream& detail::operator<< <Blocks...>(std::ostream&, value_type const&);

    template<std::size_t... N>
    Size size_impl(std::index_sequence<N...>) const {
        return {
            detail::plus_fold(
                detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                    ::size(std::get<N>(blocks_)).x...),
            std::max({ detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                ::size(std::get<N>(blocks_)).y... }),
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators begins(std::index_sequence<N...>) const {
        return typename value_type::block_iterators{
            detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                ::begin(std::get<N>(blocks_))...
        };
    }

    template<std::size_t... N>
    typename value_type::block_iterators ends(std::index_sequence<N...>) const {
        return typename value_type::block_iterators{
            detail::block_traits<std::tuple_element_t<N, decltype(blocks_)>>
                ::end(std::get<N>(blocks_))...
        };
    }

    std::size_t margin_ = 2;
    std::tuple<Blocks...> blocks_;
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
