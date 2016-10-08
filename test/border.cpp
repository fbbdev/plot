#include "../terminal_canvas.hpp"

#include <cmath>
#include <iostream>
#include <string>

using namespace plot;

int main() {
    TerminalCanvas canvas({ 30, 7 }, TerminalColor::Xterm24bit);

    Rect rect({ 0, 0 }, canvas.size() - Point(1, 2));
    auto size = rect.size() + Point(1, 1);

    auto y0 = rect.p1.y, A = size.y/2, N = size.x;
    float f = 2;

    canvas.stroke({ 0.2f, 0.2f, 1.0f }, rect, [y0,A,N,f](float x) {
              Coord base = y0 + A - std::lround(A*std::sin(2*3.141592f*f*x/N)),
                    end  = y0 + A - std::lround(A*std::sin(2*3.141592f*f*(x+1)/N));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          })
          .stroke({ 1.0f, 0.4f, 0.4f }, rect, [y0,A,N,f](float x) {
              Coord base = y0 + A - std::lround(A*std::cos(2*3.141592f*f*x/N)),
                    end  = y0 + A - std::lround(A*std::cos(2*3.141592f*f*(x+1)/N));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          })
          .line({ 1.0f, 1.0f, 1.0f }, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

    std::cout << "\n  \x1b[0m┌";

    for (int i = 0; i < canvas.term_size().x; ++i)
        std::cout << "─";

    std::cout << "┐\n";

    for (auto const& line: canvas) {
        std::cout << "  \x1b[0m│" << line << "│\n";
    }

    std::cout << "  \x1b[0m└";

    for (int i = 0; i < canvas.term_size().x; ++i)
        std::cout << "─";

    std::cout << "┘\n" << std::endl;

    return 0;
}
