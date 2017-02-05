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

using namespace plot;

int main() {
    BrailleCanvas canvas({ 70, 20 }, TerminalInfo().detect());

    canvas.rect({ 1.0f, 0.35f, 0.0f }, { 0.0f, 0.2f, 1.0f }, { { 11, 11 }, { 40, 40 } })
          .push()
              .line({ 0.4f,  1.0f, 0.4f }, { 12, 17 }, { 17, 39 })
              .line({ 0.4f,  1.0f, 0.4f }, { 17, 39 }, { 39, 34 })
              .line({ 0.4f,  1.0f, 0.4f }, { 39, 34 }, { 34, 12 })
              .line({ 0.4f,  1.0f, 0.4f }, { 34, 12 }, { 12, 17 })
          .pop(TerminalOp::ClipDst)
          .ellipse({ 0.6f, 0.6f, 0.6f }, Rect({ 30, 30 }) + Point(45, 11))
          .ellipse({ 0, 0, 0 }, { 1.0f, 1.0f, 1.0f }, { 60, 26 }, { 10, 10 })
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
