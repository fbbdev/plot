#include "../braille.hpp"
#include "../layout.hpp"

#include <cmath>
#include <iostream>
#include <string>

using namespace plot;

int main() {
    TerminalInfo term;
    term.detect();

    BrailleCanvas canvas({ 30, 7 }, term);

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
          .line(term.foreground_color, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

    std::cout << margin(frame(&canvas, term)) << std::flush;

    return 0;
}
