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

#include "plot.hpp"

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

    std::cout << margin(1, 2, 0, 2, frame(BorderStyle::None, &canvas, term))
              << margin(1, 2, 0, 2, frame(u8"Label", &canvas, term))
              << margin(1, 2, 0, 2, frame(u8"Label", Align::Center, { BorderStyle::Solid, true }, &canvas, term))
              << margin(1, 2, 0, 2, frame(u8"Label", Align::Right, BorderStyle::SolidBold, &canvas, term))
              << margin(1, 2, 0, 2, frame(BorderStyle::Dashed, &canvas, term))
              << margin(1, 2, 0, 2, frame({ BorderStyle::Dashed, true }, &canvas, term))
              << margin(1, 2, 0, 2, frame(BorderStyle::DashedBold, &canvas, term))
              << margin(1, 2, 0, 2, frame(BorderStyle::Dotted, &canvas, term))
              << margin(1, 2, 0, 2, frame({ BorderStyle::Dotted, true }, &canvas, term))
              << margin(1, 2, 0, 2, frame(BorderStyle::DottedBold, &canvas, term))
              << margin(frame(BorderStyle::Double, &canvas, term)) << std::flush;

    return 0;
}
