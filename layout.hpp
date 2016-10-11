#pragma once

#include "point.hpp"
#include "string_view.hpp"
#include "terminal.hpp"
#include "utils.hpp"

#include <iomanip>
#include <iterator>
#include <ostream>
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
    struct block_ref_traits
    {
        using iterator = single_line_adapter<Block>;

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
    struct block_ref_traits<Block, true>
    {
        using iterator = typename Block::const_iterator;

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
                top = bottom = "╌";
                left = right = "┆";
                break;
            case BorderStyle::DashedBold:
                top = bottom = "╍";
                left = right = "┇";
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
            : margin(margin), overflow(overflow), line(line), end(end)
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
        if (!line.overflow && line.line != line.end)
            stream << std::setfill(' ')
                   << std::setw(line.margin->left)
                   << ""
                   << *line.line
                   << std::setw(line.margin->right)
                   << ""
                   << std::setfill(fill);

        return stream;
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
        : block(block)
        {}

    explicit Margin(Block block, Coord left, Coord right, Coord top, Coord bottom)
        : left(left), right(right), top(top), bottom(bottom), block(block)
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
        return { { this, -top, detail::block_traits<Block>::begin(block), detail::block_traits<Block>::end(block) } };
    }

    const_iterator cend() const {
        return { { this, bottom, detail::block_traits<Block>::end(block), detail::block_traits<Block>::end(block) } };
    }

private:
    friend std::ostream& detail::operator<< <Block>(std::ostream&, value_type const&);

    Coord left = 2, right = 2, top = 1, bottom = 1;
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
inline Margin<std::decay_t<Block>> margin(Block&& block, Coord left, Coord right, Coord top, Coord bottom) {
    return Margin<std::decay_t<Block>>(std::forward<Block>(block), left, right, top, bottom);
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
            : frame(frame), overflow(overflow), line(line), end(end)
            {}

        frame_line next() const {
            return (overflow || line == end) ? frame_line(frame, overflow + 1, line, end)
                                             : frame_line(frame, overflow, std::next(line), end);
        }

        bool equal(frame_line const& other) const {
            return line == other.line && overflow == other.overflow && frame == other.frame;
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
        : block(block), term(term)
        {}

    explicit Frame(Block block, Border border, TerminalInfo term = TerminalInfo())
        : border(border), block(block), term(term)
        {}

    explicit Frame(Block block, string_view label, TerminalInfo term = TerminalInfo())
        : label(label), block(block), term(term)
        {}

    explicit Frame(Block block, string_view label, Align align, TerminalInfo term = TerminalInfo())
        : label(label), align(align), block(block), term(term)
        {}

    explicit Frame(Block block, string_view label, Border border, TerminalInfo term = TerminalInfo())
        : label(label), border(border), block(block), term(term)
        {}

    explicit Frame(Block block, string_view label, Align align, Border border, TerminalInfo term = TerminalInfo())
        : label(label), align(align), border(border), block(block), term(term)
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
inline Frame<std::decay_t<Block>> frame(Block&& block, Border border, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(std::forward<Block>(block), border, term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(Block&& block, string_view label, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(std::forward<Block>(block), label, term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(Block&& block, string_view label, Align align, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(std::forward<Block>(block), label, align, term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(Block&& block, string_view label, Border border, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(std::forward<Block>(block), label, border, term);
}

template<typename Block>
inline Frame<std::decay_t<Block>> frame(Block&& block, string_view label, Align align, Border border, TerminalInfo term = TerminalInfo()) {
    return Frame<std::decay_t<Block>>(std::forward<Block>(block), label, align, border, term);
}

} /* namespace plot */
