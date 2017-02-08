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

#include "../plot.hpp"

#include <cmath>
#include <iostream>
#include "opengl_colors.hpp"

using namespace plot;

int main() {
    constexpr Coord canvasCols=70;
    constexpr Coord canvasRows=20;
    constexpr Size canvasSize{canvasCols,canvasRows};
    BrailleCanvas canvas(ColorPicker::floralwhite,canvasSize, TerminalInfo().detect());

    canvas.rect(ColorPicker::firebrick, ColorPicker::blueviolet, { { 11, 11 }, { 40, 40 } })
          .push()
              .line(ColorPicker::limegreen, { 12, 17 }, { 17, 39 })
              .line(ColorPicker::limegreen, { 17, 39 }, { 39, 34 })
              .line(ColorPicker::limegreen, { 39, 34 }, { 34, 12 })
              .line(ColorPicker::limegreen, { 34, 12 }, { 12, 17 })
          .pop(TerminalOp::ClipDst);
    canvas.ellipse(ColorPicker::grey, Rect({ 30, 30 }) + Point(45, 11))
          .ellipse(ColorPicker::black, ColorPicker::white, { 60, 26 }, { 10, 10 });

    canvas.stroke(ColorPicker::mediumblue, { { 12, 42 }, { 71, 67 } }, [](Coord x) {
              Coord base = 55 - std::lround(10*std::sin(2*3.141592f*((x - 12)/30.0f))),
                    end  = 55 - std::lround(10*std::sin(2*3.141592f*((x - 11)/30.0f)));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          });
    canvas.stroke(ColorPicker::crimson, { { 12, 42 }, { 71, 67 } }, [](Coord x) {
              Coord base = 55 - std::lround(10*std::cos(2*3.141592f*((x - 12)/30.0f))),
                    end  = 55 - std::lround(10*std::cos(2*3.141592f*((x - 11)/30.0f)));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          }, TerminalOp::ClipSrc);

    std::cout << canvas << std::endl;

    return 0;
}
