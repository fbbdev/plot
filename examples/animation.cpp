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

#include "iterators.hpp"

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

    RealCanvas<BrailleCanvas> canvas({ { 0.0f, 1.0f }, { 1.0f, -1.0f } }, Size(30, 7), term);
    auto layout = margin(frame(&canvas, term));

    auto bounds = canvas.bounds();
    auto size = canvas.size();
    auto pixel = canvas.unmap_size({ 1, 1 });

    float A = size.y/2.0f;
    float f = 2.0f;

    auto sin = [A,f](float t) {
        return A*std::sin(2*3.141592f*f*t);
    };

    auto cos = [A,f](float t) {
        return A*std::cos(2*3.141592f*f*t);
    };

    auto plot_fn = [](auto const& fn, float t_) {
        return [&fn, t_](float x) -> Pointf {
            return { x, fn(t_ + x) };
        };
    };

    // Plot function in range [bounds.p1.x, bounds.p2.x] with step of 1px
    // Actually, the range is [bounds.p1.x, bounds.p2.x + 1px)
    range_iterator<float> rng(bounds.p1.x, bounds.p2.x + pixel.x, pixel.x);
    range_iterator<float> rng_end;

    float t = 0.0f;

    while (true) {
        canvas.clear()
              .path(palette::royalblue, map(rng, plot_fn(sin, t)), map(rng_end, plot_fn(sin, t)))
              .path(palette::red, map(rng, plot_fn(cos, t)), map(rng_end, plot_fn(cos, t)))
              .line(term.foreground_color, { bounds.p1.x, 0.0f }, { bounds.p2.x, 0.0f }, TerminalOp::ClipSrc);

        for (auto const& line: layout)
            std::cout << term.clear_line() << line << '\n';

        std::cout << std::flush;

        if (!run)
            break;

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(40ms);

        if (!run)
            break;

        t += 0.01f;
        if (t >= 1.0f)
            t -= std::trunc(t);

        std::cout << term.move_up(layout.size().y) << std::flush;
    }

    return 0;
}
