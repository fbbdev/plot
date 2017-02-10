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

#include "terminal.hpp"

#include <iostream>

using namespace plot;

inline std::ostream& operator<<(std::ostream& stream, TerminalMode mode) {
    stream << u8"TerminalMode::";

    switch (mode) {
        case TerminalMode::None:
            return stream << u8"None";
        case TerminalMode::Minimal:
            return stream << u8"Minimal";
        case TerminalMode::Ansi:
            return stream << u8"Ansi";
        case TerminalMode::Ansi256:
            return stream << u8"Ansi256";
        case TerminalMode::Iso24bit:
            return stream << u8"Iso24bit";
        case TerminalMode::Windows:
            return stream << u8"Windows";
    }

    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Color const& c) {
    return stream << u8"Color{ " << c.r << u8", " << c.g << u8", " << c.b << u8", " << c.a << u8" }";
}

inline std::ostream& operator<<(std::ostream& stream, Point const& p) {
    return stream << u8"{ " << p.x << u8", " << p.y << u8" }";
}

int main() {
    TerminalInfo info;

    std::cout << std::boolalpha;

    std::cout << u8"     is_terminal: " << info.is_terminal() << '\n'
              << u8"            mode: " << info.mode << '\n'
              << u8"foreground_color: " << info.foreground_color << '\n'
              << u8"background_color: " << info.background_color << '\n'
              << u8"            size: " << info.size() << '\n'
              << std::endl;

    std::cout << u8"             loc: " << info.cursor() << '\n' << std::endl;

    info.detect();

    std::cout << u8"     is_terminal: " << info.is_terminal() << '\n'
              << u8"            mode: " << info.mode << '\n'
              << u8"foreground_color: " << info.foreground_color << '\n'
              << u8"background_color: " << info.background_color << '\n'
              << u8"            size: " << info.size() << '\n'
              << std::endl;

    std::cout << u8"             loc: " << info.cursor() << '\n' << std::endl;

    std::cout << u8"\n\n\n" << info.move_up(3)
              << info.move_down(2)
              << info.foreground({ 1, 0.3, 0 }) << u8"before!!\n" << info.reset()
              << info.move_up(3)
              << info.background({ 1, 0.3, 0 }) << u8"after!!" << info.reset()
              << info.move_down(3) << std::endl;

    std::cout << info.move_forward(12) << info.bold() << u8"before!!" << info.reset()
              << info.move_backward(20) << u8"after!!\n" << std::endl;

    std::cout << u8"set title" << info.title(u8"Terminal handling test") << std::flush;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    std::cout << info.line_start() << u8"Hello" << std::flush;

    auto loc = info.cursor();

    std::this_thread::sleep_for(2s);

    std::cout << info.foreground(ansi::Color::Green)
              << info.background(ansi::Color::Gray)
              << u8" there" << info.reset() << std::flush;

    std::this_thread::sleep_for(2s);

    std::cout << info.move_to(loc) << u8" you fools" << std::flush;

    std::this_thread::sleep_for(2s);

    std::cout << info.line_start() << info.clear_line() << u8"'morning" << std::endl;

    std::this_thread::sleep_for(2s);

    std::cout << info.clear() << std::flush;

    return 0;
}
