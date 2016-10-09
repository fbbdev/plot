#include "../braille.hpp"

#include <cmath>
#include <iostream>

using namespace plot;

int main() {
    TerminalCanvas canvas({ 70, 20 }, TerminalColor::Iso24bit);

    canvas.rect({ 1.0f, 0.35f, 0.0f }, { 0.0f, 0.2f, 1.0f }, { { 11, 11 }, { 40, 40 } })
          .line({ 0.4f,  1.0f, 0.4f }, { 12, 12 }, { 39, 39 }, TerminalOp::ClipDst)
          .ellipse({ 0.6f, 0.6f, 0.6f }, Rect({ 30, 30 }) + Point(45, 11))
          .ellipse({ 0, 0, 0 }, { 1.0f, 1.0f, 1.0f }, Rect({ 20, 20 }) + Point(50, 16))
          .stroke({ 0.2f, 0.2f, 1.0f }, { { 12, 42 }, { 71, 67 } }, [](Coord x) {
              Coord base = 55 - std::lround(10*std::sin(2*3.141592f*((x - 12)/30.0f))),
                    end  = 55 - std::lround(10*std::sin(2*3.141592f*((x - 11)/30.0f)));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          })
          .stroke({ 1.0f, 0.4f, 0.4f }, { { 12, 42 }, { 71, 67 } }, [](Coord x) {
              Coord base = 55 - std::lround(10*std::cos(2*3.141592f*((x - 12)/30.0f))),
                    end  = 55 - std::lround(10*std::cos(2*3.141592f*((x - 11)/30.0f)));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          }, TerminalOp::ClipSrc);

    std::cout << canvas << std::endl;

    return 0;
}
