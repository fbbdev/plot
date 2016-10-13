#pragma once

#include "color.hpp"
#include "point.hpp"
#include "rect.hpp"
#include "utils.hpp"

#include <cmath>
#include <type_traits>

namespace plot
{

template<typename Canvas>
class RealCanvas
{
public:
    using point_type = Pointf;
    using size_type = Sizef;
    using rect_type = Rectf;

    RealCanvas() = default;

    template<typename Arg, typename... Args, std::enable_if_t<!std::is_same<std::decay_t<Arg>, Rectf>::value>* = nullptr>
    RealCanvas(Arg&& arg, Args&&... args)
        : canvas_(std::forward<Arg>(arg), std::forward<Args>(args)...)
        {}

    template<typename... Args>
    RealCanvas(Rectf bounds, Args&&... args)
        : bounds_(bounds), canvas_(std::forward<Args>(args)...)
        {}

    Canvas& canvas() {
        return canvas_;
    }

    Canvas const& canvas() const {
        return canvas_;
    }

    Rectf bounds() const {
        return bounds_;
    }

    Sizef size() const {
        return bounds_.size();
    }

    RealCanvas& push() {
        canvas_.push();
        return *this;
    }

    template<typename... Args>
    RealCanvas& pop(Args&&... args) {
        canvas_.pop(std::forward<Args>(args)...);
        return *this;
    }

    template<typename Size>
    RealCanvas& resize(Size&& size) {
        canvas_.resize(std::forward<Size>(size));
        return *this;
    }

    template<typename Size>
    RealCanvas& resize(Rectf bounds, Size&& size) {
        canvas_.resize(std::forward<Size>(size));
        bounds_ = bounds;
        return *this;
    }

    RealCanvas& clear() {
        canvas_.clear();
        return *this;
    }

    RealCanvas& clear(Color background) {
        canvas_.clear(background);
        return *this;
    }

    RealCanvas& clear(Rectf rect) {
        canvas_.clear(map(rect));
        return *this;
    }

    template<typename... Args>
    RealCanvas& dot(Color const& color, Pointf p, Args&&... args) {
        canvas_.dot(color, map(p), std::forward<Args>(args)...);
        return *this;
    }

    template<typename... Args>
    RealCanvas& line(Color const& color, Pointf from, Pointf to, Args&&... args) {
        canvas_.line(color, map(from), map(to), std::forward<Args>(args)...);
        return *this;
    }

    template<typename Iterator, typename... Args>
    RealCanvas& path(Color const& color, Iterator first, Iterator last, Args&&... args) {
        push();
        auto start = *first;
        while (++first != last) {
            auto end = *first;
            line(color, start, end);
            start = end;
        }
        return pop(std::forward<Args>(args)...);
    }

    template<typename... Args>
    RealCanvas& path(Color const& color, std::initializer_list<Pointf> const& points, Args&&... args) {
        return path(color, points.begin(), points.end(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    RealCanvas& rect(Color const& color, Rectf const& rect, Args&&... args) {
        canvas_.rect(color, map(rect), std::forward<Args>(args)...);
        return *this;
    }

    template<typename... Args>
    RealCanvas& rect(Color const& stroke, Color const& fill, Rectf const& rect, Args&&... args) {
        canvas_.rect(stroke, fill, map(rect), std::forward<Args>(args)...);
        return *this;
    }

    template<typename... Args>
    RealCanvas& ellipse(Color const& color, Rectf const& rect, Args&&... args) {
        canvas_.ellipse(color, map(rect), std::forward<Args>(args)...);
        return *this;
    }

    template<typename... Args>
    RealCanvas& ellipse(Color const& stroke, Color const& fill, Rectf const& rect, Args&&... args) {
        canvas_.ellipse(stroke, fill, map(rect), std::forward<Args>(args)...);
        return *this;
    }

    template<typename... Args>
    RealCanvas& ellipse(Color const& color, Pointf center, Sizef semiaxes, Args&&... args) {
        canvas_.ellipse(color, map(center), map_size(semiaxes), std::forward<Args>(args)...);
        return *this;
    }

    template<typename... Args>
    RealCanvas& ellipse(Color const& color, Color const& fill, Pointf center, Sizef semiaxes, Args&&... args) {
        canvas_.ellipse(color, fill, map(center), map_size(semiaxes), std::forward<Args>(args)...);
        return *this;
    }

    typename Canvas::point_type map(Pointf const& p) const {
        auto canvas_bounds = canvas_.size() - Point(1, 1);
        return {
            std::lround((p.x - bounds_.p1.x)/(bounds_.p2.x - bounds_.p1.x) * canvas_bounds.x),
            std::lround((p.y - bounds_.p1.y)/(bounds_.p2.y - bounds_.p1.y) * canvas_bounds.y)
        };
    }

    typename Canvas::rect_type map(Rectf const& r) const {
        return { map(r.p1), map(r.p2) };
    }

    typename Canvas::size_type map_size(Sizef const& s) const {
        auto size = this->size();
        auto canvas_bounds = canvas_.size() - Point(1, 1);
        return {
            std::lround(s.x/size.x * canvas_bounds.x),
            std::lround(s.y/size.y * canvas_bounds.y)
        };
    }

    Pointf unmap(typename Canvas::point_type const& p) const {
        auto canvas_bounds = canvas_.size() - Point(1, 1);
        return {
            (float(p.x)/canvas_bounds.x)*(bounds_.p2.x - bounds_.p1.x) + bounds_.p1.x,
            (float(p.y)/canvas_bounds.y)*(bounds_.p2.y - bounds_.p1.y) + bounds_.p1.y
        };
    }

    Rectf unmap(typename Canvas::rect_type const& r) const {
        return { unmap(r.p1), unmap(r.p2) };
    }

    Sizef unmap_size(typename Canvas::size_type const& s) const {
        auto size = this->size();
        auto canvas_bounds = canvas_.size() - Point(1, 1);
        return {
            float(s.x)/canvas_bounds.x * size.x,
            float(s.y)/canvas_bounds.y * size.y
        };
    }

private:
    Rectf bounds_{ { 0.0f, 1.0f }, { 1.0f, 0.0f } };
    Canvas canvas_;
};

template<typename Canvas>
inline std::ostream& operator<<(std::ostream& stream, RealCanvas<Canvas> const& canvas) {
    return stream << canvas.canvas();
}

} /* namespace plot */
