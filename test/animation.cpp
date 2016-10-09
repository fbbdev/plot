#include "../braille.hpp"

#include <cmath>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace plot;

int main() {
    TerminalCanvas canvas({ 30, 7 }, TerminalColor::Iso24bit);

    Rect rect({ 0, 0 }, canvas.size() - Point(1, 2));
    auto size = rect.size() + Point(1, 1);

    auto y0 = rect.p1.y, A = size.y/2, N = size.x;
    float f = 2;

    float t = 0.0f;

    auto sin = [N,f](float t, float x) {
        return std::sin(2*3.141592f*f*(t + x/N));
    };

    auto cos = [N,f](float t, float x) {
        return std::cos(2*3.141592f*f*(t + x/N));
    };

    auto stroke_fn = [y0,A](auto const& fn, float t) {
        return [y0,A,fn,t](float x) {
            Coord base = y0 + A - std::lround(A*fn(t, x)),
                  end  = y0 + A - std::lround(A*fn(t, x + 1));
            return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
        };
    };

    while (true) {
        canvas.clear()
              .stroke({ 0.2f, 0.2f, 1.0f }, rect, stroke_fn(sin, t))
              .stroke({ 1.0f, 0.4f, 0.4f }, rect, stroke_fn(cos, t))
              .line({ 1.0f, 1.0f, 1.0f }, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

        std::cout << "\x1b[K\n\x1b[K  \x1b[0m┌";

        for (int i = 0; i < canvas.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┐\n";

        for (auto const& line: canvas) {
            std::cout << "\x1b[K  \x1b[0m│" << line << "│\n";
        }

        std::cout << "\x1b[K  \x1b[0m└";

        for (int i = 0; i < canvas.term_size().x; ++i)
            std::cout << "─";

        std::cout << "┘\n" << std::endl;

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(40ms);

        t += 0.01f;
        if (t >= 1.0f)
            t -= std::trunc(t);

        std::cout << "\x1b[" << canvas.term_size().y + 4 << "A" << std::flush;
    }
    return 0;
}
