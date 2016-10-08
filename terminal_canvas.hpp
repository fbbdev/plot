#pragma once

#include "color.hpp"
#include "point.hpp"
#include "rect.hpp"
#include "utils.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <forward_list>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace plot
{

enum class TerminalColor {
    None,
    Minimal,
    Ansi,
    Xterm256,
    Xterm24bit
};

enum class TerminalOp {
    Over,
    ClipDst,
    ClipSrc
};


namespace detail
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

    using ansi_color_t = std::pair<std::uint8_t, bool>;
    using ansi_palette_entry_t = std::pair<Color, ansi_color_t>;

    static constexpr ansi_palette_entry_t ansi_palette[16] = {
        { { 0, 0, 0 }, { 0, false } },                                      // Black
        { { 170.0f/255.0f, 0, 0 }, { 1, false } },                          // Red
        { { 0, 170.0f/255.0f, 0 }, { 2, false } },                          // Green
        { { 170.0f/255.0f, 85.0f/255.0f, 0 }, { 3, false } },               // Brown
        { { 0, 0, 170.0f/255.0f }, { 4, false } },                          // Blue
        { { 170.0f/255.0f, 0, 170.0f/255.0f }, { 5, false } },              // Magenta
        { { 0, 170.0f/255.0f, 170.0f/255.0f }, { 6, false } },              // Cyan
        { { 170.0f/255.0f, 170.0f/255.0f, 170.0f/255.0f }, { 7, false } },  // Gray
        { { 85.0f/255.0f, 85.0f/255.0f, 85.0f/255.0f }, { 0, true } },      // Darkgray
        { { 1.0f, 85.0f/255.0f, 85.0f/255.0f }, { 1, true } },              // Bright Red
        { { 85.0f/255.0f, 1.0f, 85.0f/255.0f }, { 2, true } },              // Bright green
        { { 1.0f, 1.0f, 85.0f/255.0f }, { 3, true } },                      // Yellow
        { { 85.0f/255.0f, 85.0f/255.0f, 1.0f }, { 4, true } },              // Bright Blue
        { { 1.0f, 85.0f/255.0f, 1.0f }, { 5, true } },                      // Bright Magenta
        { { 85.0f/255.0f, 1.0f, 1.0f }, { 6, true } },                      // Bright Cyan
        { { 1.0f, 1.0f, 1.0f }, { 7, true } }                               // White
    };

    inline ansi_palette_entry_t find_ansi_entry(Color c) {
        return *std::min_element(ansi_palette, ansi_palette + 16, [c](auto const& e1, auto const& e2) {
            return e1.first.distance(c) < e2.first.distance(c);
        });
    }

    inline ansi_color_t to_ansi(Color c) {
        return find_ansi_entry(c).second;
    }

    inline std::uint8_t to_xterm256(Color c) {
        using utils::clamp;
        auto ansi_entry = find_ansi_entry(c);
        auto color = c.color32(5);
        std::uint8_t gray = std::lround(clamp(0.3f*c.r + 0.59f*c.g + 0.11f*c.b, 0.0f, 1.0f)*23);

        auto ansi_dist = ansi_entry.first.distance(c);
        auto color_dist = Color(color, 5).distance(c);
        auto gray_dist = Color({ gray, gray, gray, 255 }, 23).distance(c);

        if (color_dist <= gray_dist && color_dist <= ansi_dist) {
            return 16 + 36*color.r + 6*color.g + color.b;
        } else if (gray_dist <= ansi_dist) {
            return gray + 0xe8;
        } else {
            return ansi_entry.second.first + 8*ansi_entry.second.second;
        }
    }

    class image_t : public std::vector<detail::block_t>
    {
        using base = std::vector<detail::block_t>;

    public:
        using base::base;

        image_t(Size size)
            : base(size.y*size.x)
            {}

        void clear() {
            assign(size(), detail::block_t());
        }

        void resize(Size from, Size to) {
            if (std::size_t(to.y*to.x) > size())
                resize(to.y*to.x, detail::block_t{});

            auto first = begin();

            if (to.x < from.x) {
                for (Coord line = 1, end = std::min(to.y, from.y); line < end; ++line)
                    std::copy(first + line*from.x, first + line*from.x + to.x, first + line*to.x);
            } else if (to.x > from.x) {
                for (Coord line = std::min(to.y, from.y) - 1; line > 0; --line) {
                    std::copy_backward(first + line*from.x, first + line*from.x + from.x, first + line*to.x + from.x);
                    std::fill(first + line*from.x, first + line*to.x, detail::block_t{});
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
} /* namespace detail */


class TerminalCanvas {
public:
    using value_type = std::string const;
    using reference = std::string const&;
    using const_reference = std::string const&;

    class const_iterator {
    public:
        using value_type = std::string const;
        using reference = std::string const&;
        using pointer = std::string const*;
        using difference_type = Coord;
        using iterator_category = std::random_access_iterator_tag;

        const_iterator() = default;

        reference operator*() const {
            return data;
        }

        pointer operator->() const {
            return &data;
        }

        const_iterator& operator++() {
            line += canvas->cols;
            fetch_data();
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator prev = std::move(*this);
            line += canvas->cols;
            fetch_data();
            return prev;
        }

        const_iterator& operator--() {
            line -= canvas->cols;
            fetch_data();
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator prev = std::move(*this);
            line -= canvas->cols;
            fetch_data();
            return prev;
        }

        const_iterator operator+(difference_type n) const {
            return { canvas, line + n*difference_type(canvas->cols) };
        }

        const_iterator& operator+=(difference_type n) {
            line += n*difference_type(canvas->cols);
            fetch_data();
            return *this;
        }

        const_iterator operator-(difference_type n) const {
            return { canvas, line - n*difference_type(canvas->cols) };
        }

        const_iterator& operator-=(difference_type n) {
            line -= n*difference_type(canvas->cols);
            fetch_data();
            return *this;
        }

        value_type operator[](difference_type n) const {
            return (*this + n).data;
        }

        difference_type operator-(const_iterator const& other) const {
            return (line - other.line)/canvas->cols;
        }

        bool operator==(const_iterator const& other) const {
            return line == other.line;
        }

        bool operator!=(const_iterator const& other) const {
            return line != other.line;
        }

        bool operator<(const_iterator const& other) const {
            return line < other.line;
        }

        bool operator<=(const_iterator const& other) const {
            return line <= other.line;
        }

        bool operator>(const_iterator const& other) const {
            return line > other.line;
        }

        bool operator>=(const_iterator const& other) const {
            return line >= other.line;
        }

    private:
        friend class TerminalCanvas;

        const_iterator(TerminalCanvas const* canvas, detail::image_t::const_iterator line)
            : canvas(canvas), line(line)
        {
            fetch_data();
        }

        // XXX: Not a real template, just to avoid declaring this inline
        template<typename = void>
        void fetch_data();

        TerminalCanvas const* canvas = nullptr;
        detail::image_t::const_iterator line{};
        std::string data{};
    };

    using iterator = const_iterator;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    TerminalCanvas() = default;

    TerminalCanvas(Size term_size, TerminalColor mode = TerminalColor::None)
        : lines(term_size.y), cols(term_size.x), blocks(term_size), mode(mode)
    {
        available_layers.emplace_front(term_size);
    }

    TerminalCanvas(Color background, Size term_size, TerminalColor mode = TerminalColor::None)
        : lines(term_size.y), cols(term_size.x), blocks(term_size), background(background), mode(mode)
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

    TerminalCanvas& push() {
        if (available_layers.empty())
            available_layers.emplace_front(term_size());

        stack.splice_after(stack.before_begin(), available_layers, available_layers.before_begin());
        blocks.swap(stack.front());
        blocks.clear();
        return *this;
    }

    TerminalCanvas& pop(TerminalOp op = TerminalOp::Over) {
        if (!stack.empty()) {
            stack.front().paint(blocks, op);
            blocks.swap(stack.front());
            available_layers.splice_after(available_layers.before_begin(), stack, stack.before_begin());
        }
        return *this;
    }

    TerminalCanvas& resize(Size size) {
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

    TerminalCanvas& clear() {
        blocks.clear();
        return *this;
    }

    TerminalCanvas& clear(Color background) {
        this->background = background;
        return clear();
    }

    TerminalCanvas& clear(Rect rect) {
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
                detail::block_t src({ 0, 0, 0, 0 },
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
    TerminalCanvas& stroke(Color const& color, Rect rect, Fn&& fn, TerminalOp op = TerminalOp::Over);

    template<typename Fn>
    TerminalCanvas& fill(Color const& color, Rect rect, Fn&& fn, TerminalOp op = TerminalOp::Over);

    TerminalCanvas& dot(Color const& color, Point p, TerminalOp op = TerminalOp::Over) {
        if (Rect({}, size()).contains(p)) {
            paint(p.y / 4, p.x / 2, detail::block_t(color).set(p.x % 2, p.y % 4), op);
        }
        return *this;
    }

    TerminalCanvas& line(Color const& color, Point from, Point to, TerminalOp op = TerminalOp::Over) {
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
    TerminalCanvas& path(Color const& color, Iterator first, Iterator last, TerminalOp op = TerminalOp::Over) {
        push();
        auto start = *first;
        while (++first != last) {
            auto end = *first;
            line(color, start, end, TerminalOp::Over);
            start = end;
        }
        return pop(op);
    }

    TerminalCanvas& path(Color const& color, std::initializer_list<Point> const& points, TerminalOp op = TerminalOp::Over) {
        return path(color, points.begin(), points.end(), op);
    }

    TerminalCanvas& rect(Color const& color, Rect const& rect, TerminalOp op = TerminalOp::Over) {
        return push()
              .line(color, rect.p1, { rect.p2.x, rect.p1.y }, TerminalOp::Over)
              .line(color, rect.p1, { rect.p1.x, rect.p2.y }, TerminalOp::Over)
              .line(color, rect.p2, { rect.p2.x, rect.p1.y }, TerminalOp::Over)
              .line(color, rect.p2, { rect.p1.x, rect.p2.y }, TerminalOp::Over)
              .pop(op);
    }

    TerminalCanvas& rect(Color const& stroke, Color const& fill, Rect rect, TerminalOp op = TerminalOp::Over) {
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

    TerminalCanvas& ellipse(Color const& color, Rect rect, TerminalOp op = TerminalOp::Over) {
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

    TerminalCanvas& ellipse(Color const& stroke, Color const& fill, Rect rect, TerminalOp op = TerminalOp::Over) {
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
    friend class TerminalCanvas::const_iterator;

    detail::block_t& block(std::size_t line, std::size_t col) {
        return blocks[cols*line + col];
    }

    detail::block_t const& block(std::size_t line, std::size_t col) const {
        return blocks[cols*line + col];
    }

    detail::block_t& paint(std::size_t line, std::size_t col, detail::block_t const& src, TerminalOp op) {
        auto& dst = block(line, col);
        return dst = src.paint(dst, op);
    }

    std::size_t lines = 0, cols = 0;
    detail::image_t blocks;

    std::forward_list<detail::image_t> stack;
    std::forward_list<detail::image_t> available_layers;

    Color background = { 0, 0, 0, 1 };
    TerminalColor mode = TerminalColor::None;
};

template<typename Fn>
TerminalCanvas& TerminalCanvas::stroke(Color const& color, Rect rect, Fn&& fn, TerminalOp op) {
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

            detail::block_t src(color);

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
TerminalCanvas& TerminalCanvas::fill(Color const& color, Rect rect, Fn&& fn, TerminalOp op) {
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
            detail::block_t src(color,
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

template<typename>
void TerminalCanvas::const_iterator::fetch_data() {
    data.clear();

    // Reset attributes + Bold mode
    // XXX: Braille patterns are not rendered correctly in normal mode
    if (canvas->mode != TerminalColor::None)
        data.append("\x1b[0;1m");

    // Unicode braille patterns are 0x28xx
    // In binary:
    //   0b00101000'xxxxxxxx
    // In UTF-8:
    //   0b1110'0010, 0b10'1000'xx 0b10'xxxxxx
    std::uint8_t block[3] = { 0b1110'0010, 0, 0 };

    char buffer[32] = {};

    Color color;
    Color32 c32;
    detail::ansi_color_t ansi_color;

    for (auto it = line, end = line+canvas->cols; it != end; ++it) {
        color = it->color.over(canvas->background).premultiplied();

        switch (canvas->mode) {
            case TerminalColor::Ansi:
                ansi_color = detail::to_ansi(color);
                std::sprintf(buffer, "\x1b[%dm", 30 + ansi_color.first);
                break;
            case TerminalColor::Xterm256:
                std::sprintf(buffer, "\x1b[38;5;%dm", detail::to_xterm256(color));
                break;
            case TerminalColor::Xterm24bit:
                c32 = color.color32();
                std::sprintf(buffer, "\x1b[38;2;%d;%d;%dm", c32.r, c32.g, c32.b);
                break;
            default:
                break;
        }

        if (it->pixels) {
            data.append(buffer);

            block[1] = 0b10'1000'00 | ((it->pixels & 0b11'000000) >> 6);
            block[2] = 0b10'000000 | (it->pixels & 0b00'111111);
            data.append((const char*) block, 3);
        } else {
            data.append(1, ' ');
        }
    }

    // Reset terminal attributes
    if (canvas->mode != TerminalColor::None)
        data.append("\x1b[0m");
}

TerminalCanvas::const_iterator operator+(TerminalCanvas::const_iterator::difference_type n,
                                         TerminalCanvas::const_iterator const& it) {
    return it + n;
}

std::ostream& operator<<(std::ostream& stream, TerminalCanvas const& canvas) {
    for (auto const& line: canvas) {
        stream << "\x1b[K" << line << '\n';
    }

    return stream;
}

};
