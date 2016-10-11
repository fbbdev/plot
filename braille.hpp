#pragma once

#include "color.hpp"
#include "point.hpp"
#include "rect.hpp"
#include "terminal.hpp"
#include "utils.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <forward_list>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace plot
{

class BrailleCanvas;

namespace detail { namespace braille
{
    // Unicode braille patterns: 0x28xx
    // See https://en.wikipedia.org/wiki/Braille_Patterns
    static constexpr std::uint8_t pixel_codes[2][4] = { { 0x01, 0x02, 0x04, 0x40 }, { 0x08, 0x10, 0x20, 0x80 } };

    inline constexpr std::uint8_t bitcount(std::uint8_t n) {
        return (n & 1) + bool(n & 2) + bool(n & 4) + bool(n & 8) +
               bool(n & 16) + bool(n & 32) + bool(n & 64) + bool(n & 128);
    }

    struct block_t {
        constexpr block_t() = default;

        constexpr block_t(Color color, bool px00, bool px01, bool px02, bool px03,
                                       bool px10, bool px11, bool px12, bool px13)
            : color(color),
              pixels(pixel_codes[0][0]*px00 | pixel_codes[0][1]*px01 |
                     pixel_codes[0][2]*px02 | pixel_codes[0][3]*px03 |
                     pixel_codes[1][0]*px10 | pixel_codes[1][1]*px11 |
                     pixel_codes[1][2]*px12 | pixel_codes[1][3]*px13)
            {}

        constexpr block_t(Color color, std::uint8_t pixels = 0)
            : color(color), pixels(pixels)
            {}

        block_t& clear() {
            pixels = 0;
            return *this;
        }

        block_t& clear(std::size_t x, std::size_t y) {
            pixels &= ~pixel_codes[x % 2][y % 4];
            return *this;
        }

        block_t& set(std::size_t x, std::size_t y) {
            pixels |= pixel_codes[x % 2][y % 4];
            return *this;
        }

        constexpr block_t over(block_t const& other) const {
            auto old = bitcount(other.pixels & ~pixels);
            auto new_ = bitcount(pixels & ~other.pixels);

            std::uint8_t over_pixels = other.pixels & pixels;
            auto over = bitcount(over_pixels);

            float total = old + new_ + over;

            auto old_color = (other.color.a != 0.0f) ? other.color : color;
            auto new_color = (color.a != 0.0f) ? color : other.color;
            auto over_color = new_color.over(old_color);

            auto mixed_color = (old/total)*old_color + (new_/total)*new_color + (over/total)*over_color;

            return { mixed_color, std::uint8_t(pixels | other.pixels) };
        }

        constexpr block_t paint(block_t const& dst, TerminalOp op) const {
            if (pixels) {
                switch (op) {
                    case TerminalOp::Over:
                        return over(dst);
                    case TerminalOp::ClipDst:
                        return *this;
                    case TerminalOp::ClipSrc:
                        if (!dst.pixels)
                            return *this;
                }
            }

            return dst;
        }

        constexpr block_t operator~() const {
            return { color, std::uint8_t(~pixels) };
        }

        constexpr block_t operator|(block_t const& other) const {
            return { color, std::uint8_t(pixels | other.pixels) };
        }

        block_t& operator|=(block_t const& other) {
            return (*this) = (*this) | other;
        }

        constexpr block_t operator&(block_t const& other) const {
            return { color, std::uint8_t(pixels & other.pixels) };
        }

        block_t& operator&=(block_t const& other) {
            return (*this) = (*this) & other;
        }

        Color color{ 0, 0, 0, 0 };
        std::uint8_t pixels = 0;
    };

    class image_t : public std::vector<block_t>
    {
        using base = std::vector<block_t>;

    public:
        using base::base;

        image_t(Size size)
            : base(size.y*size.x)
            {}

        void clear() {
            assign(size(), block_t());
        }

        void resize(Size from, Size to) {
            if (std::size_t(to.y*to.x) > size())
                resize(to.y*to.x, block_t());

            auto first = begin();

            if (to.x < from.x) {
                for (Coord line = 1, end = std::min(to.y, from.y); line < end; ++line)
                    std::copy(first + line*from.x, first + line*from.x + to.x, first + line*to.x);
            } else if (to.x > from.x) {
                for (Coord line = std::min(to.y, from.y) - 1; line > 0; --line) {
                    std::copy_backward(first + line*from.x, first + line*from.x + from.x, first + line*to.x + from.x);
                    std::fill(first + line*from.x, first + line*to.x, block_t());
                }
            }

            if (std::size_t(to.y*to.x) < size())
                resize(to.y*to.x);
        }

        // XXX: undefined behavior if this and other do not have the same layout
        void paint(image_t const& other, TerminalOp op) {
            auto dst = begin();
            for (auto src = other.begin(), src_end = other.end(); src != src_end; ++src, ++dst)
                *dst = src->paint(*dst, op);
        }

    private:
        using base::resize;
    };

    class line_t;

    template<typename = void>
    std::ostream& operator<<(std::ostream& stream, line_t const& line);

    class line_t {
        friend class iterator;
        template<typename>
        friend std::ostream& operator<<(std::ostream&, line_t const&);

        line_t(BrailleCanvas const* canvas, image_t::const_iterator it)
            : canvas(canvas), it(it)
            {}

        line_t move(image_t::const_iterator::difference_type n) const;

        image_t::const_iterator::difference_type distance(line_t const& other) const;

        BrailleCanvas const* canvas;
        image_t::const_iterator it;
    };

    class iterator {
    public:
        using value_type = line_t;
        using reference = value_type const&;
        using pointer = value_type const*;
        using difference_type = image_t::const_iterator::difference_type;
        using iterator_category = std::random_access_iterator_tag;

        iterator() = default;

        reference operator*() const {
            return line;
        }

        pointer operator->() const {
            return &line;
        }

        iterator& operator++() {
            line = line.move(1);
            return *this;
        }

        iterator operator++(int) {
            iterator prev = std::move(*this);
            line = line.move(1);
            return prev;
        }

        iterator& operator--() {
            line = line.move(-1);
            return *this;
        }

        iterator operator--(int) {
            iterator prev = std::move(*this);
            line = line.move(-1);
            return prev;
        }

        iterator operator+(difference_type n) const {
            return { line.move(n) };
        }

        iterator& operator+=(difference_type n) {
            line = line.move(n);
            return *this;
        }

        iterator operator-(difference_type n) const {
            return { line.move(-n) };
        }

        iterator& operator-=(difference_type n) {
            line = line.move(-n);
            return *this;
        }

        value_type operator[](difference_type n) const {
            return line.move(n);
        }

        difference_type operator-(iterator const& other) const {
            return line.distance(other.line);
        }

        bool operator==(iterator const& other) const {
            return line.it == other.line.it;
        }

        bool operator!=(iterator const& other) const {
            return line.it != other.line.it;
        }

        bool operator<(iterator const& other) const {
            return line.it < other.line.it;
        }

        bool operator<=(iterator const& other) const {
            return line.it <= other.line.it;
        }

        bool operator>(iterator const& other) const {
            return line.it > other.line.it;
        }

        bool operator>=(iterator const& other) const {
            return line.it >= other.line.it;
        }

    private:
        friend class plot::BrailleCanvas;

        iterator(BrailleCanvas const* canvas, image_t::const_iterator it)
            : line(canvas, it)
            {}

        iterator(line_t line)
            : line(line)
            {}

        line_t line;
    };

    inline iterator operator+(iterator::difference_type n, iterator const& it) {
        return it + n;
    }
} /* namespace braille */ } /* namespace detail */


class BrailleCanvas {
public:
    using value_type = detail::braille::line_t;
    using reference = value_type const&;
    using const_reference = value_type const&;
    using const_iterator = detail::braille::iterator;
    using iterator = const_iterator;
    using difference_type = const_iterator::difference_type;
    using size_type = std::size_t;

    BrailleCanvas() = default;

    BrailleCanvas(Size term_size, TerminalInfo term = TerminalInfo())
        : lines(term_size.y), cols(term_size.x), blocks(term_size),
          background(term.background_color), term(term)
    {
        available_layers.emplace_front(term_size);
    }

    BrailleCanvas(Color background, Size term_size, TerminalInfo term = TerminalInfo())
        : lines(term_size.y), cols(term_size.x), blocks(term_size),
          background(background), term(term)
    {
        available_layers.emplace_front(term_size);
    }

    Size term_size() const {
        return { Coord(cols), Coord(lines) };
    }

    Size size() const {
        return { Coord(2*cols), Coord(4*lines) };
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { this, blocks.cbegin() };
    }

    const_iterator cend() const {
        return { this, blocks.cend() };
    }

    BrailleCanvas& push() {
        if (available_layers.empty())
            available_layers.emplace_front(term_size());

        stack.splice_after(stack.before_begin(), available_layers, available_layers.before_begin());
        blocks.swap(stack.front());
        blocks.clear();
        return *this;
    }

    BrailleCanvas& pop(TerminalOp op = TerminalOp::Over) {
        if (!stack.empty()) {
            stack.front().paint(blocks, op);
            blocks.swap(stack.front());
            available_layers.splice_after(available_layers.before_begin(), stack, stack.before_begin());
        }
        return *this;
    }

    BrailleCanvas& resize(Size size) {
        if (size != term_size()) {
            blocks.resize(term_size(), size);

            for (auto& layer: stack)
                layer.resize(term_size(), size);

            if (!available_layers.empty()) {
                available_layers.clear();
                available_layers.emplace_front(size);
            }

            lines = size.y; cols = size.x;
        }
        return *this;
    }

    BrailleCanvas& clear() {
        blocks.clear();
        return *this;
    }

    BrailleCanvas& clear(Color background) {
        this->background = background;
        return clear();
    }

    BrailleCanvas& clear(Rect rect) {
        rect = rect.sorted();
        Rect block_rect{
            { rect.p1.x/2, rect.p1.y/4 },
            { utils::max(1l, rect.p2.x/2 + (rect.p2.x%2)),
              utils::max(1l, rect.p2.y/4 + (rect.p2.y%4 != 0)) }
        };

        rect.p2 += Point(1, 1);

        for (auto line = block_rect.p1.y; line < block_rect.p2.y; ++line) {
            auto ybase = 4*line;
            for (auto col = block_rect.p1.x; col < block_rect.p2.x; ++col) {
                auto xbase = 2*col;
                detail::braille::block_t src({ 0, 0, 0, 0 },
                    rect.contains({ xbase, ybase }),
                    rect.contains({ xbase, ybase+1 }),
                    rect.contains({ xbase, ybase+2 }),
                    rect.contains({ xbase, ybase+3 }),
                    rect.contains({ xbase+1, ybase }),
                    rect.contains({ xbase+1, ybase+1 }),
                    rect.contains({ xbase+1, ybase+2 }),
                    rect.contains({ xbase+1, ybase+3 }));
                block(line, col) &= ~src;
            }
        }

        return *this;
    }

    template<typename Fn>
    BrailleCanvas& stroke(Color const& color, Rect rect, Fn&& fn, TerminalOp op = TerminalOp::Over);

    template<typename Fn>
    BrailleCanvas& fill(Color const& color, Rect rect, Fn&& fn, TerminalOp op = TerminalOp::Over);

    BrailleCanvas& dot(Color const& color, Point p, TerminalOp op = TerminalOp::Over) {
        if (Rect({}, size()).contains(p)) {
            paint(p.y / 4, p.x / 2, detail::braille::block_t(color).set(p.x % 2, p.y % 4), op);
        }
        return *this;
    }

    BrailleCanvas& line(Color const& color, Point from, Point to, TerminalOp op = TerminalOp::Over) {
        auto dx = to.x - from.x,
             dy = to.y - from.y;

        dx += (dx >= 0) - (dx < 0);
        dy += (dy >= 0) - (dy < 0);

        return stroke(color, { from, to }, [dx, dy, x0 = from.x, y0 = from.y](Coord x) {
            auto base = (x - x0)*dy/dx + y0,
                 end = (1 + x - x0)*dy/dx + y0;
            return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
        }, op);
    }

    template<typename Iterator>
    BrailleCanvas& path(Color const& color, Iterator first, Iterator last, TerminalOp op = TerminalOp::Over) {
        push();
        auto start = *first;
        while (++first != last) {
            auto end = *first;
            line(color, start, end, TerminalOp::Over);
            start = end;
        }
        return pop(op);
    }

    BrailleCanvas& path(Color const& color, std::initializer_list<Point> const& points, TerminalOp op = TerminalOp::Over) {
        return path(color, points.begin(), points.end(), op);
    }

    BrailleCanvas& rect(Color const& color, Rect const& rect, TerminalOp op = TerminalOp::Over) {
        return push()
              .line(color, rect.p1, { rect.p2.x, rect.p1.y }, TerminalOp::Over)
              .line(color, rect.p1, { rect.p1.x, rect.p2.y }, TerminalOp::Over)
              .line(color, rect.p2, { rect.p2.x, rect.p1.y }, TerminalOp::Over)
              .line(color, rect.p2, { rect.p1.x, rect.p2.y }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& rect(Color const& stroke, Color const& fill, Rect rect, TerminalOp op = TerminalOp::Over) {
        rect = rect.sorted();
        return push()
              .line(stroke, rect.p1, { rect.p2.x, rect.p1.y }, TerminalOp::Over)
              .line(stroke, rect.p1, { rect.p1.x, rect.p2.y }, TerminalOp::Over)
              .line(stroke, rect.p2, { rect.p2.x, rect.p1.y }, TerminalOp::Over)
              .line(stroke, rect.p2, { rect.p1.x, rect.p2.y }, TerminalOp::Over)
              .fill(fill, rect, [r=Rect(rect.p1 + Point(1, 1), rect.p2)](Point p) {
                  return r.contains(p);
              }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& ellipse(Color const& color, Rect rect, TerminalOp op = TerminalOp::Over) {
        rect = rect.sorted();
        auto size = rect.size() + Point(1, 1);

        float x_fac = 2.0f/size.x;
        Coord y_fac = size.y/2 - (!(size.y % 2)),
              cx = rect.p1.x + (size.x/2) - (!(size.x % 2)),
              cy = rect.p1.y + y_fac;

        return push()
              .stroke(color, { rect.p1, { cx, cy } }, [x_fac,y_fac,cy,x0=rect.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(color, { { cx + 1, rect.p1.y }, { rect.p2.x, cy } }, [x_fac,y_fac,cy,x1=rect.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((x1 - x + 1) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(color, { { rect.p1.x, cy + 1 }, { cx, rect.p2.y } }, [x_fac,y_fac,cy,x0=rect.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(color, { { cx + 1, cy + 1 }, rect.p2 }, [x_fac,y_fac,cy,x1=rect.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x1 - x) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& ellipse(Color const& stroke, Color const& fill, Rect rect, TerminalOp op = TerminalOp::Over) {
        rect = rect.sorted();
        auto size = rect.size() + Point(1, 1);

        float x_fac = 2.0f/size.x;
        Coord y_fac = size.y/2 - (!(size.y % 2)),
              cx = rect.p1.x + (size.x/2) - (!(size.x % 2)),
              cy = rect.p1.y + y_fac;

        return push()
              .stroke(stroke, { rect.p1, { cx, cy } }, [x_fac,y_fac,cy,x0=rect.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(stroke, { { cx + 1, rect.p1.y }, { rect.p2.x, cy } }, [x_fac,y_fac,cy,x1=rect.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((x1 - x + 1) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(stroke, { { rect.p1.x, cy + 1 }, { cx, rect.p2.y } }, [x_fac,y_fac,cy,x0=rect.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(stroke, { { cx + 1, cy + 1 }, rect.p2 }, [x_fac,y_fac,cy,x1=rect.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x1 - x) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .fill(fill, { rect.p1, { cx, cy } }, [x_fac,y_fac,cy,x0=rect.p1.x](Point p) {
                  auto x_over_a = ((p.x - x0) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y > base;
              }, TerminalOp::Over)
              .fill(fill, { { cx + 1, rect.p1.y }, { rect.p2.x, cy } }, [x_fac,y_fac,cy,x1=rect.p2.x](Point p) {
                  auto x_over_a = ((x1 - p.x) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y > base;
              }, TerminalOp::Over)
              .fill(fill, { { rect.p1.x, cy + 1 }, { cx, rect.p2.y } }, [x_fac,y_fac,cy,x0=rect.p1.x](Point p) {
                  auto x_over_a = ((p.x - x0) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y < base;
              }, TerminalOp::Over)
              .fill(fill, { { cx + 1, cy + 1 }, rect.p2 }, [x_fac,y_fac,cy,x1=rect.p2.x](Point p) {
                  auto x_over_a = ((x1 - p.x) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y < base;
              }, TerminalOp::Over)
              .pop(op);
    }

private:
    friend value_type;
    template<typename>
    friend std::ostream& detail::braille::operator<<(std::ostream&, detail::braille::line_t const&);

    detail::braille::block_t& block(std::size_t line, std::size_t col) {
        return blocks[cols*line + col];
    }

    detail::braille::block_t const& block(std::size_t line, std::size_t col) const {
        return blocks[cols*line + col];
    }

    detail::braille::block_t& paint(std::size_t line, std::size_t col,
                                    detail::braille::block_t const& src, TerminalOp op) {
        auto& dst = block(line, col);
        return dst = src.paint(dst, op);
    }

    std::size_t lines = 0, cols = 0;
    detail::braille::image_t blocks;

    std::forward_list<detail::braille::image_t> stack;
    std::forward_list<detail::braille::image_t> available_layers;

    Color background = { 0, 0, 0, 1 };
    TerminalInfo term;
};

template<typename Fn>
BrailleCanvas& BrailleCanvas::stroke(Color const& color, Rect rect, Fn&& fn, TerminalOp op) {
    auto size = this->size();
    rect = Rect(rect.p1.clamp({}, size), rect.p2.clamp({}, size)).sorted();
    rect.p2 += Point(1, 1);
    Rect block_rect{
        { rect.p1.x/2, rect.p1.y/4 },
        { utils::max(1l, rect.p2.x/2 + (rect.p2.x%2)),
          utils::max(1l, rect.p2.y/4 + (rect.p2.y%4 != 0)) }
    };

    for (auto line = block_rect.p1.y; line < block_rect.p2.y; ++line) {
        auto line_start = utils::clamp(4*line, rect.p1.y, rect.p2.y),
             line_end = utils::clamp(4*line + 4, rect.p1.y, rect.p2.y);

        for (auto col = block_rect.p1.x; col < block_rect.p2.x; ++col) {
            auto col_start = utils::clamp(2*col, rect.p1.x, rect.p2.x),
                 col_end = utils::clamp(2*col + 2, rect.p1.x, rect.p2.x);

            detail::braille::block_t src(color);

            for (auto x = col_start; x < col_end; ++x) {
                auto ybounds = fn(x);

                if (ybounds.second < ybounds.first)
                    ybounds = { ybounds.second + 1, ybounds.first + 1 };

                ybounds.first = utils::max(ybounds.first, line_start),
                ybounds.second = utils::min(ybounds.second, line_end);

                for (auto y = ybounds.first; y < ybounds.second; ++y)
                    src.set(x, y);
            }

            paint(line, col, src, op);
        }
    }

    return *this;
}

template<typename Fn>
BrailleCanvas& BrailleCanvas::fill(Color const& color, Rect rect, Fn&& fn, TerminalOp op) {
    rect = rect.sorted();
    rect.p2 += Point(1, 1);
    Rect block_rect{
        { rect.p1.x/2, rect.p1.y/4 },
        { utils::max(1l, rect.p2.x/2 + (rect.p2.x%2)),
          utils::max(1l, rect.p2.y/4 + (rect.p2.y%4 != 0)) }
    };

    auto set = [rect,&fn](Point p) {
        return rect.contains(p) && fn(p);
    };

    for (auto line = block_rect.p1.y; line < block_rect.p2.y; ++line) {
        auto ybase = 4*line;
        for (auto col = block_rect.p1.x; col < block_rect.p2.x; ++col) {
            auto xbase = 2*col;
            detail::braille::block_t src(color,
                set({ xbase, ybase }),
                set({ xbase, ybase+1 }),
                set({ xbase, ybase+2 }),
                set({ xbase, ybase+3 }),
                set({ xbase+1, ybase }),
                set({ xbase+1, ybase+1 }),
                set({ xbase+1, ybase+2 }),
                set({ xbase+1, ybase+3 }));

            paint(line, col, src, op);
        }
    }

    return *this;
}


inline std::ostream& operator<<(std::ostream& stream, BrailleCanvas const& canvas) {
    for (auto const& line: canvas)
        stream << line << '\n';

    return stream;
}


namespace detail { namespace braille
{
    inline line_t line_t::move(image_t::const_iterator::difference_type n) const {
        return { canvas, it + n*image_t::const_iterator::difference_type(canvas->cols) };
    }

    inline image_t::const_iterator::difference_type line_t::distance(line_t const& other) const {
        return (other.it - it) / image_t::const_iterator::difference_type(canvas->cols);
    }

    template<typename>
    std::ostream& operator<<(std::ostream& stream, line_t const& line) {
        auto const& canvas = *line.canvas;
        auto const& term = canvas.term;

        // Reset attributes + Bold mode
        // XXX: Empty dots in braille patterns are often rendered as empty
        // XXX: circles unless in bold mode.
        stream << term.reset() << term.bold();

        // Unicode braille patterns are 0x28xx
        // In binary:
        //   0b00101000'xxxxxxxx
        // In UTF-8:
        //   0b1110'0010, 0b10'1000'xx 0b10'xxxxxx

        for (auto it = line.it, end = line.it+canvas.cols; it != end; ++it) {
            if (it->pixels) {
                stream << term.foreground(it->color.over(canvas.background).premultiplied())
                       << char(0b1110'0010)
                       << char(0b10'1000'00 | ((it->pixels & 0b11'000000) >> 6))
                       << char(0b10'000000 | (it->pixels & 0b00'111111));
            } else {
                stream << ' ';
            }
        }

        return stream << term.reset();
    }
} /* namespace braille */ } /* namespace detail */

} /* namespace plot */
