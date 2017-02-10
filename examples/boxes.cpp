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

    RealCanvas<BrailleCanvas> waves({ { 0.0f, 1.0f }, { 1.0f, -1.0f } }, Size(30, 7), term);
    RealCanvas<BrailleCanvas> mul_waves(waves.bounds(), waves.canvas().char_size(), term);
    RealCanvas<BrailleCanvas> circle(
        { { -1.2f, 1.2f }, { 1.2f, -1.2f } },
        Size(2*(waves.canvas().char_size().y + mul_waves.canvas().char_size().y + 3),
             waves.canvas().char_size().y + mul_waves.canvas().char_size().y + 3),
        term);

    // Build block layout
    auto layout =
        margin(
            hbox(
                vbox(
                    frame(u8"cos(t), sin(t)", Align::Center, &waves),
                    frame(u8"cos(t)·sin(t), sin²(t)", Align::Center, &mul_waves)),
                frame(u8"P(cos(t)·sin(t), sin²(t))", Align::Center, &circle)));

    auto bounds = waves.bounds();
    auto size = waves.size();
    auto pixel = waves.unmap_size({ 1, 1 });

    auto circle_bounds = circle.bounds();

    float A = size.y/2.0f;
    float f = 2.0f;

    int track_length = (size.x/pixel.x)/(2*f)/2;

    auto sin = [A,f](float t) {
        return A*std::sin(2*3.141592f*f*t);
    };

    auto cos = [A,f](float t) {
        return A*std::cos(2*3.141592f*f*t);
    };

    auto sin2 = [sin](float t) {
        auto val = sin(t);
        return val*val;
    };

    auto sincos = [sin,cos](float t) {
        return sin(t) * cos(t);
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

    constexpr Color sin_color(0.2f, 0.2f, 1.0f);
    constexpr Color cos_color(1.0f, 0.4f, 0.4f);
    constexpr Color sin2_color(0.4f, 1.0f, 0.4f);
    constexpr Color sincos_color(1.0f, 0.8f, 0.2f);

    float t = 0.0f;

    while (true) {
        waves.clear()
             .path(sin_color, map(rng, plot_fn(sin, t)), map(rng_end, plot_fn(sin, t)))
             .path(cos_color, map(rng, plot_fn(cos, t)), map(rng_end, plot_fn(cos, t)))
             .line(term.foreground_color, { bounds.p1.x, 0.0f }, { bounds.p2.x, 0.0f }, TerminalOp::ClipSrc);

        mul_waves.clear()
                 .path(sin2_color, map(rng, plot_fn(sin2, t)), map(rng_end, plot_fn(sin2, t)))
                 .path(sincos_color, map(rng, plot_fn(sincos, t)), map(rng_end, plot_fn(sincos, t)))
                 .line(term.foreground_color, { bounds.p1.x, 0.0f }, { bounds.p2.x, 0.0f }, TerminalOp::ClipSrc);

        Pointf pos(sincos(t + bounds.p2.x), sin2(t + bounds.p2.x));

        circle.clear()
              // X axis
              .line(term.foreground_color, { circle_bounds.p1.x, 0 }, { circle_bounds.p2.x, 0 })
              // Y axis
              .line(term.foreground_color, { 0, circle_bounds.p1.y }, { 0, circle_bounds.p2.y })
              // pos.x component
              .line(sincos_color, { 0, pos.y }, pos)
              // pos.y component
              .line(sin2_color, { pos.x, 0 }, pos)
              // radius
              .line(term.foreground_color, { 0, 0 }, pos)
              // Draw small cross at pos
              .push()
                  .dot(term.foreground_color, pos)
                  .dot(term.foreground_color, pos - circle.unmap_size({ 1, 0 }))
                  .dot(term.foreground_color, pos + circle.unmap_size({ 1, 0 }))
                  .dot(term.foreground_color, pos - circle.unmap_size({ 0, 1 }))
                  .dot(term.foreground_color, pos + circle.unmap_size({ 0, 1 }))
              .pop()
              .push();

        for (int i = 0; i < track_length; ++i) {
            auto x = i*pixel.x;
            Pointf start(sincos(t + (bounds.p2.x - x)), sin2(t + (bounds.p2.x - x)));
            Pointf end(sincos(t + (bounds.p2.x - x - pixel.x)), sin2(t + (bounds.p2.x - x - pixel.x)));
            circle.line(term.foreground_color.alpha(float(track_length - i)/track_length), start, end);
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
