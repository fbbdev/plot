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

#include "../braille.hpp"
#include "../real_canvas.hpp"
#include "../layout.hpp"

#include <cmath>
#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace plot;

static volatile std::sig_atomic_t run = true;

int main() {
    std::signal(SIGINT, [](int) {
        run = false;
    });

    TerminalInfo term;
    term.detect();

    BrailleCanvas waves({ 30, 7 }, term);
    BrailleCanvas mul_waves(waves.char_size(), term);
    RealCanvas<BrailleCanvas> circle(
        { { -1.2f, 1.2f }, { 1.2f, -1.2f } },
        Size(2*(waves.char_size().y + mul_waves.char_size().y + 3),
             waves.char_size().y + mul_waves.char_size().y + 3),
        term);

    auto layout = margin(hbox(vbox(frame(&waves), frame(&mul_waves)), frame(&circle)));

    Rect rect({ 0, 0 }, waves.size() - Point(1, 2));
    auto size = rect.size() + Point(1, 1);

    auto circle_bounds = circle.bounds();

    auto y0 = rect.p1.y, A = size.y/2, N = size.x;
    float f = 2.0f;


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

    float t = 0.0f;
    while (true) {
        waves.clear()
             .stroke({ 0.2f, 0.2f, 1.0f }, rect, stroke_fn(sin, t))
             .stroke({ 1.0f, 0.4f, 0.4f }, rect, stroke_fn(cos, t))
             .line(term.foreground_color, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

        mul_waves.clear()
                 .stroke({ 0.4f, 1.0f, 0.4f }, rect, stroke_fn(sin2, t))
                 .stroke({ 1.0f, 0.8f, 0.2f }, rect, stroke_fn(sincos, t))
                 .line(term.foreground_color, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

        Pointf pos(sincos(t, N), sin2(t, N));

        circle.clear()
              .line(term.foreground_color, { circle_bounds.p1.x, 0 }, { circle_bounds.p2.x, 0 })
              .line(term.foreground_color, { 0, circle_bounds.p1.y }, { 0, circle_bounds.p2.y })
              .line({ 1.0f, 0.8f, 0.2f }, { 0, pos.y }, pos)
              .line({ 0.4f, 1.0f, 0.4f }, { pos.x, 0 }, pos)
              .line(term.foreground_color, { 0, 0 }, pos)
              .push()
                  .dot(term.foreground_color, pos)
                  .dot(term.foreground_color, pos - circle.unmap_size({ 1, 0 }))
                  .dot(term.foreground_color, pos + circle.unmap_size({ 1, 0 }))
                  .dot(term.foreground_color, pos - circle.unmap_size({ 0, 1 }))
                  .dot(term.foreground_color, pos + circle.unmap_size({ 0, 1 }))
              .pop()
              .push();

        auto track_length = N/Coord(2*f)/2;
        for (Coord x = 0; x < track_length; ++x) {
            Pointf start(sincos(t, N - x), sin2(t, N - x));
            Pointf end(sincos(t, N - x - 1), sin2(t, N - x - 1));
            circle.line(term.foreground_color.alpha(float(track_length - x)/track_length), start, end);
        }

        circle.pop();

        std::cout << layout << std::flush;

        if (!run)
            break;

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(40ms);

        if (!run)
            break;

        t += 0.007f;
        if (t >= 1.0f)
            t -= std::trunc(t);

        std::cout << term.move_up(layout.size().y) << std::flush;
    }

    return 0;
}
