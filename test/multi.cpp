#include "../terminal_canvas.hpp"

#include <cmath>
#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace plot;

volatile static std::sig_atomic_t run = true;

int main() {
    std::signal(SIGINT, [](int) {
        run = false;
    });

    TerminalCanvas waves({ 30, 7 }, TerminalColor::Xterm24bit);
    TerminalCanvas circle({ 34, 17 }, TerminalColor::Xterm24bit);

    Rect rect({ 0, 0 }, waves.size() - Point(1, 2));
    auto size = rect.size() + Point(1, 1);

    Rect circle_rect({ 0, 0 }, circle.size() - Point(2, 2));
    auto circle_size = circle_rect.size() + Point(1, 1);
    Point circle_center(circle_size.x/2, circle_size.y/2);
    auto circle_radius = circle_center - Point(4, 4);

    auto y0 = rect.p1.y, A = size.y/2, N = size.x;
    float f = 2;

    float t = 0.0f;

    auto sin = [N,f](float t, float x) {
        return std::sin(2*3.141592f*f*(t + x/N));
    };

    auto cos = [N,f](float t, float x) {
        return std::cos(2*3.141592f*f*(t + x/N));
    };

    auto sin2 = [sin](float t, float x) {
        auto val = sin(t, x);
        return val*val;
    };

    auto sincos = [sin,cos](float t, float x) {
        return sin(t, x) * cos(t, x);
    };

    auto stroke_fn = [y0,A](auto const& fn, float t) {
        return [y0,A,fn,t](float x) {
            Coord base = y0 + A - std::lround(A*fn(t, x)),
                  end  = y0 + A - std::lround(A*fn(t, x + 1));
            return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
        };
    };

    while (true) {
        waves.clear()
             .stroke({ 0.2f, 0.2f, 1.0f }, rect, stroke_fn(sin, t))
             .stroke({ 1.0f, 0.4f, 0.4f }, rect, stroke_fn(cos, t))
             .line({ 1.0f, 1.0f, 1.0f }, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

        Point pos(
            circle_center.x + std::lround(circle_radius.x*sincos(t, N)),
            circle_center.y - std::lround(circle_radius.y*sin2(t, N)));

        circle.clear()
              .line({ 1.0f, 1.0f, 1.0f }, { circle_center.x, 0 }, { circle_center.x, circle_rect.p2.y })
              .line({ 1.0f, 1.0f, 1.0f }, { 0, circle_center.y }, { circle_rect.p2.x, circle_center.y })
              .line({ 1.0f, 0.8f, 0.2f }, { circle_center.x, pos.y }, pos)
              .line({ 0.4f, 1.0f, 0.4f }, { pos.x, circle_center.y }, pos)
              .line({ 1.0f, 1.0f, 1.0f }, circle_center, pos)
              .ellipse({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { pos - Point(1, 1), pos + Point(1, 1) })
              .push();

        auto track_length = N/Coord(2*f)/2;
        for (Coord x = 0; x < track_length; ++x) {
            Point pos(
                circle_center.x + std::lround(circle_radius.x*sincos(t, N - x)),
                circle_center.y - std::lround(circle_radius.y*sin2(t, N - x)));
            Point prev(
                circle_center.x + std::lround(circle_radius.x*sincos(t, N - x - 1)),
                circle_center.y - std::lround(circle_radius.y*sin2(t, N - x - 1)));
            circle.line(Color(1.0f, 1.0f, 1.0f, (float(track_length - x)/track_length)), pos, prev);
        }

        circle.pop();

        std::cout << "\x1b[K\n\x1b[K  \x1b[0m┌";

        for (int i = 0; i < waves.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┐  ┌";

        for (int i = 0; i < circle.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┐\n";

        auto circle_it = circle.begin();

        for (auto const& line: waves) {
            std::cout << "\x1b[K  \x1b[0m│" << line << "│  │" << *circle_it << "│\n";
            ++circle_it;
        }

        std::cout << "\x1b[K  \x1b[0m└";

        for (int i = 0; i < waves.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┘  │" << *circle_it << "│\n";
        ++circle_it;
        std::cout << "\x1b[K   " << std::string(waves.term_size().x, ' ') << "   │" << *circle_it << "│\n";
        ++circle_it;

        waves.clear()
             .stroke({ 0.4f, 1.0f, 0.4f }, rect, stroke_fn(sin2, t))
             .stroke({ 1.0f, 0.8f, 0.2f }, rect, stroke_fn(sincos, t))
             .line({ 1.0f, 1.0f, 1.0f }, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

        std::cout << "\x1b[K  \x1b[0m┌";

        for (int i = 0; i < waves.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┐  │" << *circle_it << "│\n";
        ++circle_it;

        for (auto const& line: waves) {
            std::cout << "\x1b[K  \x1b[0m│" << line << "│  │" << *circle_it << "│\n";
            ++circle_it;
        }

        std::cout << "\x1b[K  \x1b[0m└";

        for (int i = 0; i < waves.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┘  └";

        for (int i = 0; i < circle.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┘\n" << std::endl;

        if (!run)
            break;

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(40ms);

        if (!run)
            break;

        t += 0.007f;
        if (t >= 1.0f)
            t -= std::trunc(t);

        std::cout << "\x1b[" << (2*(waves.term_size().y + 2) + 3) << "A" << std::flush;
    }

    return 0;
}
