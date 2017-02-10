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

using namespace plot;

int main() {
    RealCanvas<BrailleCanvas> canvas({ { 0.0f, 0.0f }, { 1.0f, 0.57f } }, Size(70, 20), TerminalInfo().detect());

    canvas.rect({ 1.0f, 0.35f, 0.0f }, { 0.0f, 0.2f, 1.0f }, { { 0.079f, 0.079f }, { 0.288f, 0.288f } })
          .push()
              .line({ 0.4f,  1.0f, 0.4f }, { 0.086f, 0.122f }, { 0.122f, 0.281f })
              .line({ 0.4f,  1.0f, 0.4f }, { 0.122f, 0.281f }, { 0.281f, 0.245f })
              .line({ 0.4f,  1.0f, 0.4f }, { 0.281f, 0.245f }, { 0.245f, 0.086f })
              .line({ 0.4f,  1.0f, 0.4f }, { 0.245f, 0.086f }, { 0.086f, 0.122f })
          .pop(TerminalOp::ClipDst)
          .ellipse({ 0.6f, 0.6f, 0.6f }, Rectf({ 0.214f, 0.214f }) + Pointf(0.321f, 0.079f))
          .ellipse({ 0, 0, 0 }, { 1.0f, 1.0f, 1.0f }, { 0.432f, 0.186f }, { 0.072f, 0.072f })
          .stroke({ 0.2f, 0.2f, 1.0f }, { { 0.086f, 0.3f }, { 0.507f, 0.479f } }, [](Coordf x0, Coordf x1) {
              return std::make_pair(0.393f - 0.072f*std::sin(2*3.141592f*((x0 - 0.086f)/0.211f)),
                                    0.393f - 0.072f*std::sin(2*3.141592f*((x1 - 0.086f)/0.211f)));
          })
          .stroke({ 1.0f, 0.4f, 0.4f }, { { 0.086f, 0.3f }, { 0.507f, 0.479f } }, [](Coordf x0, Coordf x1) {
              return std::make_pair(0.393f - 0.072f*std::cos(2*3.141592f*((x0 - 0.086f)/0.211f)),
                                    0.393f - 0.072f*std::cos(2*3.141592f*((x1 - 0.086f)/0.211f)));
          }, TerminalOp::ClipSrc);

    std::cout << canvas << std::endl;

    return 0;
}
