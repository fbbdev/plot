#include "../terminal.hpp"

#include <iostream>

using namespace plot;

inline std::ostream& operator<<(std::ostream& stream, TerminalMode mode) {
    stream << "TerminalMode::";

    switch (mode) {
        case TerminalMode::None:
            return stream << "None";
        case TerminalMode::Minimal:
            return stream << "Minimal";
        case TerminalMode::Ansi:
            return stream << "Ansi";
        case TerminalMode::Ansi256:
            return stream << "Ansi256";
        case TerminalMode::Iso24bit:
            return stream << "Iso24bit";
        case TerminalMode::Windows:
            return stream << "Windows";
    }

    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Color const& c) {
    return stream << "Color{ " << c.r << ", " << c.g << ", " << c.b << ", " << c.a << " }";
}

inline std::ostream& operator<<(std::ostream& stream, Point const& p) {
    return stream << "{ " << p.x << ", " << p.y << " }";
}

int main() {
    TerminalInfo info;

    std::cout << std::boolalpha;

    std::cout << "     is_terminal: " << info.is_terminal() << '\n'
              << "            mode: " << info.mode << '\n'
              << "foreground_color: " << info.foreground_color << '\n'
              << "background_color: " << info.background_color << '\n'
              << "            size: " << info.size() << '\n'
              << std::endl;

    std::cout << "             loc: " << info.cursor() << '\n' << std::endl;

    info.detect();

    std::cout << "     is_terminal: " << info.is_terminal() << '\n'
              << "            mode: " << info.mode << '\n'
              << "foreground_color: " << info.foreground_color << '\n'
              << "background_color: " << info.background_color << '\n'
              << "            size: " << info.size() << '\n'
              << std::endl;

    std::cout << "             loc: " << info.cursor() << '\n' << std::endl;

    std::cout << "\n\n\n" << info.move_up(3)
              << info.move_down(2)
              << info.foreground({ 1, 0.3, 0 }) << "before!!\n" << info.reset()
              << info.move_up(3)
              << info.background({ 1, 0.3, 0 }) << "after!!" << info.reset()
              << info.move_down(3) << std::endl;

    std::cout << info.move_forward(12) << info.bold() << "before!!" << info.reset()
              << info.move_backward(20) << "after!!\n" << std::endl;

    std::cout << "set title" << info.title("Terminal handling test") << std::flush;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    std::cout << info.line_start() << "Hello" << std::flush;

    auto loc = info.cursor();

    std::this_thread::sleep_for(2s);

    std::cout << info.foreground(ansi::Color::Green)
              << info.background(ansi::Color::Gray)
              << " there" << info.reset() << std::flush;

    std::this_thread::sleep_for(2s);

    std::cout << info.move_to(loc) << " you fools" << std::flush;

    std::this_thread::sleep_for(2s);

    std::cout << info.line_start() << info.clear_line() << "'morning" << std::endl;

    std::this_thread::sleep_for(2s);

    std::cout << info.clear() << std::flush;

    return 0;
}
