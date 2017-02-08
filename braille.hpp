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

#include "color.hpp"
#include "layout.hpp"
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
    //The dimensions of a Braille cell are 2x4
    constexpr std::uint8_t cell_cols=2;
    constexpr std::uint8_t cell_rows=4;
    // Unicode braille patterns: 0x28xx
    // See https://en.wikipedia.org/wiki/Braille_Patterns
    static constexpr std::uint8_t pixel_codes[cell_cols][cell_rows] = { { 0x01, 0x02, 0x04, 0x40 }, { 0x08, 0x10, 0x20, 0x80 } };

    inline constexpr std::uint8_t bitcount(std::uint8_t n) {
        return (n & 1) + bool(n & 2) + bool(n & 4) + bool(n & 8) +
               bool(n & 16) + bool(n & 32) + bool(n & 64) + bool(n & 128);
    }

    struct block_t {
        constexpr block_t() = default;

        constexpr block_t(Color c, bool px00, bool px01, bool px02, bool px03,
                                   bool px10, bool px11, bool px12, bool px13)
            : color(c),
              pixels(pixel_codes[0][0]*px00 | pixel_codes[0][1]*px01 |
                     pixel_codes[0][2]*px02 | pixel_codes[0][3]*px03 |
                     pixel_codes[1][0]*px10 | pixel_codes[1][1]*px11 |
                     pixel_codes[1][2]*px12 | pixel_codes[1][3]*px13)
            {}

        constexpr block_t(Color c, std::uint8_t px = 0)
            : color(c), pixels(px)
            {}

        block_t& clear() {
            pixels = 0;
            return *this;
        }

        block_t& clear(std::size_t x, std::size_t y) {
            pixels &= ~pixel_codes[x % cell_cols][y % cell_rows];
            return *this;
        }

        block_t& set(std::size_t x, std::size_t y) {
            pixels |= pixel_codes[x % cell_cols][y % cell_rows];
            return *this;
        }

        block_t over(block_t const& other) const {
            auto old = bitcount(other.pixels & ~pixels);
            auto new_ = bitcount(pixels & ~other.pixels);

            std::uint8_t over_pixels = other.pixels & pixels;
            auto over_ = bitcount(over_pixels);

            float total = old + new_ + over_;

            auto old_color = (other.color.a != 0.0f) ? other.color : color;
            auto new_color = (color.a != 0.0f) ? color : other.color;
            auto over_color = new_color.over(old_color);

            auto mixed_color = (old/total)*old_color + (new_/total)*new_color + (over_/total)*over_color;

            return { mixed_color, std::uint8_t(pixels | other.pixels) };
        }

        block_t paint(block_t const& dst, TerminalOp op) const {
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

        image_t(Size sz)
            : base(sz.y*sz.x)
            {}

        void clear() {
            assign(size(), block_t());
        }

        void resize(Size from, Size to) {
            if (std::size_t(to.y*to.x) > size())
                resize(to.y*to.x, block_t());

            auto first = begin();

            if (to.x < from.x) {
                for (Coord line = 1, end_ = std::min(to.y, from.y); line < end_; ++line)
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
        friend class detail::block_iterator<plot::BrailleCanvas, line_t>;
        friend class plot::BrailleCanvas;

        template<typename>
        friend std::ostream& operator<<(std::ostream&, line_t const&);

        line_t(BrailleCanvas const* canvas, image_t::const_iterator it)
            : canvas_(canvas), it_(it)
            {}

        line_t next() const;

        bool equal(line_t const& other) const {
            return it_ == other.it_;
        }

        BrailleCanvas const* canvas_ = nullptr;
        image_t::const_iterator it_{};

    public:
        line_t() = default;
    };
} /* namespace braille */ } /* namespace detail */


class BrailleCanvas {
public:
    constexpr static uint8_t cell_cols = detail::braille::cell_cols;
    constexpr static uint8_t cell_rows = detail::braille::cell_rows;
    using value_type = detail::braille::line_t;
    using reference = value_type const&;
    using const_reference = value_type const&;
    using const_iterator = detail::block_iterator<BrailleCanvas, value_type>;
    using iterator = const_iterator;
    using difference_type = const_iterator::difference_type;

    using coord_type = Coord;
    using point_type = Point;
    using size_type = Size;
    using rect_type = Rect;

    BrailleCanvas() = default;

    BrailleCanvas(Size char_sz, TerminalInfo term = TerminalInfo())
        : lines_(char_sz.y), cols_(char_sz.x), blocks_(char_sz),
          background_(term.background_color), term_(term)
    {
        available_layers_.emplace_front(char_sz);
    }

    BrailleCanvas(Color background, Size char_sz, TerminalInfo term = TerminalInfo())
        : lines_(char_sz.y), cols_(char_sz.x), blocks_(char_sz),
          background_(background), term_(term)
    {
        available_layers_.emplace_front(char_sz);
    }

    Size char_size() const {
        return { Coord(cols_), Coord(lines_) };
    }

    Size size() const {
        return { Coord(cell_cols*cols_), Coord(cell_rows*lines_) };
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return { { this, blocks_.cbegin() } };
    }

    const_iterator cend() const {
        return { { this, blocks_.cend() } };
    }

    BrailleCanvas& push() {
        if (available_layers_.empty())
            available_layers_.emplace_front(char_size());

        stack_.splice_after(stack_.before_begin(), available_layers_, available_layers_.before_begin());
        blocks_.swap(stack_.front());
        blocks_.clear();
        return *this;
    }

    BrailleCanvas& pop(TerminalOp op = TerminalOp::Over) {
        if (!stack_.empty()) {
            stack_.front().paint(blocks_, op);
            blocks_.swap(stack_.front());
            available_layers_.splice_after(available_layers_.before_begin(), stack_, stack_.before_begin());
        }
        return *this;
    }

    BrailleCanvas& resize(Size sz) {
        if (sz != char_size()) {
            blocks_.resize(char_size(), sz);

            for (auto& layer: stack_)
                layer.resize(char_size(), sz);

            if (!available_layers_.empty()) {
                available_layers_.clear();
                available_layers_.emplace_front(sz);
            }

            lines_ = sz.y; cols_ = sz.x;
        }
        return *this;
    }

    BrailleCanvas& clear() {
        blocks_.clear();
        return *this;
    }

    BrailleCanvas& clear(Color background) {
        this->background_ = background;
        return clear();
    }

    BrailleCanvas& clear(Rect rct) {
        rct = rct.sorted();
        Rect block_rect{
            { rct.p1.x/cell_cols, rct.p1.y/cell_rows },
            { utils::max(1l, rct.p2.x/cell_cols + (rct.p2.x%cell_cols)),
              utils::max(1l, rct.p2.y/cell_rows + (rct.p2.y%cell_rows != 0)) }
        };

        rct.p2 += Point(1, 1);

        for (auto ln = block_rect.p1.y; ln < block_rect.p2.y; ++ln) {
            auto ybase = cell_rows*ln;
            for (auto col = block_rect.p1.x; col < block_rect.p2.x; ++col) {
                auto xbase = cell_cols*col;
                detail::braille::block_t src({ 0, 0, 0, 0 },
                    rct.contains({ xbase, ybase }),
                    rct.contains({ xbase, ybase+1 }),
                    rct.contains({ xbase, ybase+2 }),
                    rct.contains({ xbase, ybase+3 }),
                    rct.contains({ xbase+1, ybase }),
                    rct.contains({ xbase+1, ybase+1 }),
                    rct.contains({ xbase+1, ybase+2 }),
                    rct.contains({ xbase+1, ybase+3 }));
                block(ln, col) &= ~src;
            }
        }

        return *this;
    }

    template<typename Fn>
    BrailleCanvas& stroke(Color const& color, Rect rct, Fn&& fn, TerminalOp op = TerminalOp::Over);

    template<typename Fn>
    BrailleCanvas& fill(Color const& color, Rect rct, Fn&& fn, TerminalOp op = TerminalOp::Over);

    BrailleCanvas& dot(Color const& color, Point p, TerminalOp op = TerminalOp::Over) {
        if (Rect({}, size()).contains(p)) {
            paint(p.y / cell_rows, p.x / cell_cols, detail::braille::block_t(color).set(p.x % cell_cols, p.y % cell_rows), op);
        }
        return *this;
    }

    BrailleCanvas& line(Color const& color, Point from, Point to, TerminalOp op = TerminalOp::Over) {
        auto sorted = Rect(from, to).sorted_x();
        auto dx = (sorted.p2.x - sorted.p1.x) + 1,
             dy = sorted.p2.y - sorted.p1.y;

        dy += (dy >= 0) - (dy < 0);

        auto gcd = utils::gcd(dx, dy);
        dx /= gcd; dy /= gcd;

        return stroke(color, sorted, [dx, dy, x0 = sorted.p1.x, y0 = sorted.p1.y](Coord x) {
            auto base = (x - x0)*dy/dx + y0,
                 end_ = (1 + x - x0)*dy/dx + y0;
            return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
        }, op);
    }

    template<typename Iterator>
    BrailleCanvas& path(Color const& color, Iterator first, Iterator last, TerminalOp op = TerminalOp::Over) {
        push();
        auto start = *first;
        while (++first != last) {
            auto end_ = *first;
            line(color, start, end_, TerminalOp::Over);
            start = end_;
        }
        return pop(op);
    }

    BrailleCanvas& path(Color const& color, std::initializer_list<Point> const& points, TerminalOp op = TerminalOp::Over) {
        return path(color, points.begin(), points.end(), op);
    }

    BrailleCanvas& rect(Color const& color, Rect const& rct, TerminalOp op = TerminalOp::Over) {
        return push()
              .line(color, rct.p1, { rct.p2.x, rct.p1.y }, TerminalOp::Over)
              .line(color, rct.p1, { rct.p1.x, rct.p2.y }, TerminalOp::Over)
              .line(color, rct.p2, { rct.p2.x, rct.p1.y }, TerminalOp::Over)
              .line(color, rct.p2, { rct.p1.x, rct.p2.y }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& rect(Color const& stroke_color, Color const& fill_color, Rect rct, TerminalOp op = TerminalOp::Over) {
        rct = rct.sorted();
        return push()
              .line(stroke_color, rct.p1, { rct.p2.x, rct.p1.y }, TerminalOp::Over)
              .line(stroke_color, rct.p1, { rct.p1.x, rct.p2.y }, TerminalOp::Over)
              .line(stroke_color, rct.p2, { rct.p2.x, rct.p1.y }, TerminalOp::Over)
              .line(stroke_color, rct.p2, { rct.p1.x, rct.p2.y }, TerminalOp::Over)
              .fill(fill_color, rct, [r=Rect(rct.p1 + Point(1, 1), rct.p2)](Point p) {
                  return r.contains(p);
              }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& ellipse(Color const& color, Rect rct, TerminalOp op = TerminalOp::Over) {
        rct = rct.sorted();
        auto size_ = rct.size() + Point(1, 1);

        float x_fac = 2.0f/size_.x;
        Coord y_fac = size_.y/2 - (!(size_.y % 2)),
              cx = rct.p1.x + (size_.x/cell_cols) - (!(size_.x % cell_cols)),
              cy = rct.p1.y + y_fac;

        return push()
              .stroke(color, { rct.p1, { cx, cy } }, [x_fac,y_fac,cy,x0=rct.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(color, { { cx + 1, rct.p1.y }, { rct.p2.x, cy } }, [x_fac,y_fac,cy,x1=rct.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((x1 - x + 1) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(color, { { rct.p1.x, cy + 1 }, { cx, rct.p2.y } }, [x_fac,y_fac,cy,x0=rct.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(color, { { cx + 1, cy + 1 }, rct.p2 }, [x_fac,y_fac,cy,x1=rct.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x1 - x) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& ellipse(Color const& stroke_color, Color const& fill_color, Rect rct, TerminalOp op = TerminalOp::Over) {
        rct = rct.sorted();
        auto size_ = rct.size() + Point(1, 1);

        float x_fac = 2.0f/size_.x;
        Coord y_fac = size_.y/2 - (!(size_.y % 2)),
              cx = rct.p1.x + (size_.x/cell_cols) - (!(size_.x % cell_cols)),
              cy = rct.p1.y + y_fac;

        return push()
              .stroke(stroke_color, { rct.p1, { cx, cy } }, [x_fac,y_fac,cy,x0=rct.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(stroke_color, { { cx + 1, rct.p1.y }, { rct.p2.x, cy } }, [x_fac,y_fac,cy,x1=rct.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((x1 - x + 1) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy - std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(stroke_color, { { rct.p1.x, cy + 1 }, { cx, rct.p2.y } }, [x_fac,y_fac,cy,x0=rct.p1.x](Coord x) {
                  auto x_over_a = ((x - x0) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x - x0) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .stroke(stroke_color, { { cx + 1, cy + 1 }, rct.p2 }, [x_fac,y_fac,cy,x1=rct.p2.x](Coord x) {
                  auto x_over_a = ((x1 - x) * x_fac) - 1.0f,
                       next_x_over_a = ((1 + x1 - x) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a)),
                        end_ = cy + std::lround(y_fac*std::sqrt(1 - next_x_over_a*next_x_over_a));
                  return (base != end_) ? std::make_pair(base, end_) : std::make_pair(base, base+1);
              }, TerminalOp::Over)
              .fill(fill_color, { rct.p1, { cx, cy } }, [x_fac,y_fac,cy,x0=rct.p1.x](Point p) {
                  auto x_over_a = ((p.x - x0) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y > base;
              }, TerminalOp::Over)
              .fill(fill_color, { { cx + 1, rct.p1.y }, { rct.p2.x, cy } }, [x_fac,y_fac,cy,x1=rct.p2.x](Point p) {
                  auto x_over_a = ((x1 - p.x) * x_fac) - 1.0f;
                  Coord base = cy - std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y > base;
              }, TerminalOp::Over)
              .fill(fill_color, { { rct.p1.x, cy + 1 }, { cx, rct.p2.y } }, [x_fac,y_fac,cy,x0=rct.p1.x](Point p) {
                  auto x_over_a = ((p.x - x0) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y < base;
              }, TerminalOp::Over)
              .fill(fill_color, { { cx + 1, cy + 1 }, rct.p2 }, [x_fac,y_fac,cy,x1=rct.p2.x](Point p) {
                  auto x_over_a = ((x1 - p.x) * x_fac) - 1.0f;
                  Coord base = cy + std::lround(y_fac*std::sqrt(1 - x_over_a*x_over_a));
                  return p.y < base;
              }, TerminalOp::Over)
              .pop(op);
    }

    BrailleCanvas& ellipse(Color const& stroke_color, Point const& center, Size const& semiaxes, TerminalOp op = TerminalOp::Over) {
        return ellipse(stroke_color, { center - semiaxes, center + semiaxes }, op);
    }

    BrailleCanvas& ellipse(Color const& stroke_color, Color const& fill_color, Point const& center, Size const& semiaxes, TerminalOp op = TerminalOp::Over) {
        return ellipse(stroke_color, fill_color, { center - semiaxes, center + semiaxes }, op);
    }

private:
    friend value_type;
    template<typename>
    friend std::ostream& detail::braille::operator<<(std::ostream&, detail::braille::line_t const&);

    detail::braille::block_t& block(std::size_t ln, std::size_t col) {
        return blocks_[cols_*ln + col];
    }

    detail::braille::block_t const& block(std::size_t ln, std::size_t col) const {
        return blocks_[cols_*ln + col];
    }

    detail::braille::block_t& paint(std::size_t ln, std::size_t col,
                                    detail::braille::block_t const& src, TerminalOp op) {
        auto& dst = block(ln, col);
        return dst = src.paint(dst, op);
    }

    std::size_t lines_ = 0, cols_ = 0;
    detail::braille::image_t blocks_;

    std::forward_list<detail::braille::image_t> stack_;
    std::forward_list<detail::braille::image_t> available_layers_;

    Color background_ = { 0, 0, 0, 1 };
    TerminalInfo term_;
};

template<typename Fn>
BrailleCanvas& BrailleCanvas::stroke(Color const& color, Rect rct, Fn&& fn, TerminalOp op) {
    rct = rct.sorted();
    rct.p2 += Point(1, 1);
    rct = rct.clamp(size());
    Rect block_rect{
        { rct.p1.x/cell_cols, rct.p1.y/cell_rows },
        { utils::max(1l, rct.p2.x/cell_cols + (rct.p2.x%cell_cols)),
          utils::max(1l, rct.p2.y/cell_rows + (rct.p2.y%cell_rows != 0)) }
    };

    for (auto ln = block_rect.p1.y; ln < block_rect.p2.y; ++ln) {
        auto line_start = utils::clamp(cell_rows*ln, rct.p1.y, rct.p2.y),
             line_end = utils::clamp(cell_rows*ln + cell_rows, rct.p1.y, rct.p2.y);

        for (auto col = block_rect.p1.x; col < block_rect.p2.x; ++col) {
            auto col_start = utils::clamp(cell_cols*col, rct.p1.x, rct.p2.x),
                 col_end = utils::clamp(cell_cols*col + cell_cols, rct.p1.x, rct.p2.x);

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

            paint(ln, col, src, op);
        }
    }

    return *this;
}

template<typename Fn>
BrailleCanvas& BrailleCanvas::fill(Color const& color, Rect rct, Fn&& fn, TerminalOp op) {
    rct = rct.sorted();
    rct.p2 += Point(1, 1);
    rct = rct.clamp(size());
    Rect block_rect{
        { rct.p1.x/cell_cols, rct.p1.y/cell_rows },
        { utils::max(1l, rct.p2.x/cell_cols + (rct.p2.x%cell_cols)),
          utils::max(1l, rct.p2.y/cell_rows + (rct.p2.y%cell_rows != 0)) }
    };

    auto set = [rct,&fn](Point p) {
        return rct.contains(p) && fn(p);
    };

    for (auto ln = block_rect.p1.y; ln < block_rect.p2.y; ++ln) {
        auto ybase = cell_rows*ln;
        for (auto col = block_rect.p1.x; col < block_rect.p2.x; ++col) {
            auto xbase = cell_cols*col;
            detail::braille::block_t src(color,
                set({ xbase, ybase }),
                set({ xbase, ybase+1 }),
                set({ xbase, ybase+2 }),
                set({ xbase, ybase+3 }),
                set({ xbase+1, ybase }),
                set({ xbase+1, ybase+1 }),
                set({ xbase+1, ybase+2 }),
                set({ xbase+1, ybase+3 }));

            paint(ln, col, src, op);
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
    inline line_t line_t::next() const {
        return { canvas_, std::next(it_, canvas_->cols_) };
    }

    template<typename>
    std::ostream& operator<<(std::ostream& stream, line_t const& line) {
        auto const& canvas = *line.canvas_;
        auto const& term = canvas.term_;

        // Reset attributes + Bold mode
        // XXX: Empty dots in braille patterns are often rendered as empty
        // XXX: circles unless in bold mode.
        stream << term.reset() << term.bold();

        // Unicode braille patterns are 0x28xx
        // In binary:
        //   0b00101000'xxxxxxxx
        // In UTF-8:
        //   0b1110'0010, 0b10'1000'xx 0b10'xxxxxx

        for (auto it = line.it_, end = line.it_+canvas.cols_; it != end; ++it) {
            if (it->pixels) {
                stream << term.foreground(it->color.over(canvas.background_).premultiplied())
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
