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

#pragma once

#include "point.hpp"
#include "color.hpp"
#include "string_view.hpp"

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <ostream>
#include <string>
#include <thread>
#include <type_traits>

#if defined(__unix__) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#define PLOT_PLATFORM_POSIX
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace plot
{

enum class TerminalMode
{
    None,       // Color not supported
    Minimal,    // Attribute reset and bold
    Ansi,       // ANSI 8-color palette
    Ansi256,    // ANSI (xterm) 256 color mode
    Iso24bit,   // ISO-8613-3 24-bit true-color mode
    Windows     // Windows console API
};

enum class TerminalOp
{
    Over,       // Paint source over destination, mix character colors
    ClipDst,    // Erase destination where source is not empty
    ClipSrc     // Ignore source where destination is not empty
};


namespace ansi
{
    namespace detail {
        using ansi_color = std::pair<int, bool>;
        using ansi_palette_entry = std::pair<plot::Color, ansi_color>;

        static constexpr ansi_palette_entry palette[16] = {
            { { 0, 0, 0 }, { 0, false } },                                      // Black
            { { 170.0f/255.0f, 0, 0 }, { 1, false } },                          // Red
            { { 0, 170.0f/255.0f, 0 }, { 2, false } },                          // Green
            { { 170.0f/255.0f, 85.0f/255.0f, 0 }, { 3, false } },               // Brown
            { { 0, 0, 170.0f/255.0f }, { 4, false } },                          // Blue
            { { 170.0f/255.0f, 0, 170.0f/255.0f }, { 5, false } },              // Magenta
            { { 0, 170.0f/255.0f, 170.0f/255.0f }, { 6, false } },              // Cyan
            { { 170.0f/255.0f, 170.0f/255.0f, 170.0f/255.0f }, { 7, false } },  // Gray
            { { 85.0f/255.0f, 85.0f/255.0f, 85.0f/255.0f }, { 0, true } },      // Darkgray
            { { 1.0f, 85.0f/255.0f, 85.0f/255.0f }, { 1, true } },              // Bright Red
            { { 85.0f/255.0f, 1.0f, 85.0f/255.0f }, { 2, true } },              // Bright green
            { { 1.0f, 1.0f, 85.0f/255.0f }, { 3, true } },                      // Yellow
            { { 85.0f/255.0f, 85.0f/255.0f, 1.0f }, { 4, true } },              // Bright Blue
            { { 1.0f, 85.0f/255.0f, 1.0f }, { 5, true } },                      // Bright Magenta
            { { 85.0f/255.0f, 1.0f, 1.0f }, { 6, true } },                      // Bright Cyan
            { { 1.0f, 1.0f, 1.0f }, { 7, true } }                               // White
        };

        inline ansi_palette_entry find_palette_entry(plot::Color c) {
            return *std::min_element(std::begin(palette), std::end(palette), [c](auto const& e1, auto const& e2) {
                return e1.first.distance(c) < e2.first.distance(c);
            });
        }

        inline ansi_color find_color(plot::Color c) {
            return find_palette_entry(c).second;
        }

        inline std::uint8_t find_color256(plot::Color c) {
            using utils::clamp;
            auto ansi_color = find_palette_entry(c);
            auto color = c.color32(5);
            std::uint8_t gray = std::lround(clamp(0.3f*c.r + 0.59f*c.g + 0.11f*c.b, 0.0f, 1.0f)*23);

            auto ansi_dist = ansi_color.first.distance(c);
            auto color_dist = plot::Color(color, 5).distance(c);
            auto gray_dist = plot::Color({ gray, gray, gray, 255 }, 23).distance(c);

            if (color_dist <= gray_dist && color_dist <= ansi_dist) {
                return 16 + 36*color.r + 6*color.g + color.b;
            } else if (gray_dist <= ansi_dist) {
                return gray + 0xe8;
            } else {
                return ansi_color.second.first + 8*ansi_color.second.second;
            }
        }

        struct title_setter
        {
            string_view title;
        };

        inline std::ostream& operator<<(std::ostream& stream, title_setter const& setter) {
            return stream << "\x1b]0;" << setter.title << "\x1b\\";
        }

        struct foreground_setter
        {
            ansi_color color;
        };

        inline std::ostream& operator<<(std::ostream& stream, foreground_setter const& setter) {
            return stream << "\x1b[" << (30 + setter.color.first) << 'm';
        }

        struct background_setter
        {
            ansi_color color;
        };

        inline std::ostream& operator<<(std::ostream& stream, background_setter const& setter) {
            return stream << "\x1b[" << (40 + setter.color.first) << 'm';
        }

        struct foreground_setter_256
        {
            std::uint8_t code;
        };

        inline std::ostream& operator<<(std::ostream& stream, foreground_setter_256 const& setter) {
            return stream << "\x1b[38;5;" << unsigned(setter.code) << 'm';
        }

        struct background_setter_256
        {
            std::uint8_t code;
        };

        inline std::ostream& operator<<(std::ostream& stream, background_setter_256 const& setter) {
            return stream << "\x1b[48;5;" << unsigned(setter.code) << 'm';
        }

        struct foreground_setter_24bit
        {
            Color32 color;
        };

        inline std::ostream& operator<<(std::ostream& stream, foreground_setter_24bit const& setter) {
            return stream << "\x1b[38;2;"
                          << unsigned(setter.color.r) << ';'
                          << unsigned(setter.color.g) << ';'
                          << unsigned(setter.color.b) << 'm';
        }

        struct background_setter_24bit
        {
            Color32 color;
        };

        inline std::ostream& operator<<(std::ostream& stream, background_setter_24bit const& setter) {
            return stream << "\x1b[48;2;"
                          << unsigned(setter.color.r) << ';'
                          << unsigned(setter.color.g) << ';'
                          << unsigned(setter.color.b) << 'm';
        }

        struct cursor_setter
        {
            Point loc;
        };

        inline std::ostream& operator<<(std::ostream& stream, cursor_setter const& setter) {
            return stream << "\x1b[" << setter.loc.y << ';' << setter.loc.x << 'H';
        }

        enum class cursor_direction
        {
            up, down, forward, backward
        };

        struct cursor_move
        {
            cursor_direction direction;
            unsigned count;
        };

        inline std::ostream& operator<<(std::ostream& stream, cursor_move const& move) {
            stream << "\x1b[" << move.count;

            switch (move.direction) {
                case cursor_direction::up:
                    return stream << 'A';
                case cursor_direction::down:
                    return stream << 'B';
                case cursor_direction::forward:
                    return stream << 'C';
                case cursor_direction::backward:
                    return stream << 'D';
            }

            return stream;
        }
    } /* namespace detail */

    enum class Color
    {
        Black = 0,
        Red = 1,
        Green = 2,
        Brown = 3,
        Blue = 4,
        Magenta = 5,
        Cyan = 6,
        Gray = 7
    };

    inline detail::title_setter title(string_view title) {
        return { title };
    }

    inline std::ostream& reset(std::ostream& stream) {
        return stream << "\x1b[0m";
    }

    inline std::ostream& bold(std::ostream& stream) {
        return stream << "\x1b[1m";
    }

    inline std::ostream& clear(std::ostream& stream) {
        return stream << "\x1b[0;0H\x1b[2J";
    }

    inline std::ostream& clear_line(std::ostream& stream) {
        return stream << "\x1b[K";
    }

    inline std::ostream& line_start(std::ostream& stream) {
        return stream << '\r';
    }

    inline detail::foreground_setter foreground(Color c) {
        return { { int(c), false } };
    }

    inline detail::background_setter background(Color c) {
        return { { int(c), false } };
    }

    inline detail::foreground_setter foreground(plot::Color c) {
        return { detail::find_color(c) };
    }

    inline detail::background_setter background(plot::Color c) {
        return { detail::find_color(c) };
    }

    inline detail::foreground_setter_256 foreground256(plot::Color c) {
        return { detail::find_color256(c) };
    }

    inline detail::background_setter_256 background256(plot::Color c) {
        return { detail::find_color256(c) };
    }

    inline detail::foreground_setter_24bit foreground24bit(plot::Color c) {
        return { c.color32() };
    }

    inline detail::background_setter_24bit background24bit(plot::Color c) {
        return { c.color32() };
    }

    inline detail::cursor_setter move_to(Point loc) {
        return { loc };
    }

    inline detail::cursor_move move_up(unsigned count = 1) {
        return { detail::cursor_direction::up, count };
    }

    inline detail::cursor_move move_down(unsigned count = 1) {
        return { detail::cursor_direction::down, count };
    }

    inline detail::cursor_move move_forward(unsigned count = 1) {
        return { detail::cursor_direction::forward, count };
    }

    inline detail::cursor_move move_backward(unsigned count = 1) {
        return { detail::cursor_direction::backward, count };
    }
} /* namespace ansi */


#ifdef PLOT_PLATFORM_POSIX

namespace detail
{
    template<typename T>
    struct ansi_manip_wrapper
    {
        TerminalMode mode;
        T manip;
    };

    template<typename T>
    inline std::ostream& operator<<(std::ostream& stream, ansi_manip_wrapper<T> const& wrapper) {
        if (wrapper.mode != TerminalMode::None && wrapper.mode != TerminalMode::Windows)
            stream << wrapper.manip;

        return stream;
    }

    template<typename T>
    inline ansi_manip_wrapper<std::decay_t<T>> make_ansi_manip_wrapper(TerminalMode mode, T&& manip) {
        return { mode, std::forward<T>(manip) };
    }

    struct foreground_setter
    {
        TerminalMode mode;
        Color color;
    };

    inline std::ostream& operator<<(std::ostream& stream, foreground_setter const& setter) {
        switch (setter.mode) {
            case TerminalMode::Ansi:
                return stream << ansi::foreground(setter.color);
            case TerminalMode::Ansi256:
                return stream << ansi::foreground256(setter.color);
            case TerminalMode::Iso24bit:
                return stream << ansi::foreground24bit(setter.color);
            default:
                return stream;
        }
    }

    struct background_setter
    {
        TerminalMode mode;
        Color color;
    };

    inline std::ostream& operator<<(std::ostream& stream, background_setter const& setter) {
        switch (setter.mode) {
            case TerminalMode::Ansi:
                return stream << ansi::background(setter.color);
            case TerminalMode::Ansi256:
                return stream << ansi::background256(setter.color);
            case TerminalMode::Iso24bit:
                return stream << ansi::background24bit(setter.color);
            default:
                return stream;
        }
    }
} /* namespace detail */

using Terminal = int;

class TerminalInfo {
public:
    explicit TerminalInfo(Terminal term = STDOUT_FILENO,
                          TerminalMode mode = TerminalMode::None,
                          Color foreground_color = { 0.9f, 0.9f, 0.9f, 1 },
                          Color background_color = { 0, 0, 0, 1 })
        : mode(mode), foreground_color(foreground_color), background_color(background_color), term(term)
        {}

    bool is_terminal() const {
        return isatty(term);
    }

    bool supported(TerminalMode mode) const {
        return int(mode) <= int(this->mode);
    }

    Size size() const {
        if (!is_terminal())
            return {};

        struct winsize ws = {};

        if (ioctl(term, TIOCGWINSZ, &ws))
            return {};

        return { ws.ws_col, ws.ws_row };
    }

    // Query cursor position. Returns { 0, 0 } when not supported.
    //
    // XXX: This will discard all pending input data and sleep for 100ms
    // XXX: to wait for a response. It is the caller's responsibility to avoid
    // XXX: negative impact on users.
    //
    // XXX: This function is not thread-safe
    Point cursor() {
        Point loc;

        if (!is_terminal())
            return loc;

        auto response = query("\x1b[6n", "R");
        if (!response.empty())
            std::sscanf(response.c_str(), "\x1b[%ld;%ldR", &loc.y, &loc.x);

        return loc;
    }

    // Detect terminal capabilities by inspecting the TERM and COLORTERM
    // environment variables. The mode property will be set only when
    // its current value is TerminalMode::None (the default).
    // If COLORTERM == "truecolor", assume 24-bit colors are supported.
    // If the terminal is compatible with xterm and the foreground_color
    // and background_color properties are set to white and black (the default),
    // query actual values by OSC 10 ; ? BEL and OSC 11 ; ? BEL
    //
    // XXX: This will discard all pending input data and sleep for 100ms
    // XXX: to wait for a response. It is the caller's responsibility to avoid
    // XXX: negative impact on users.
    //
    // XXX: This function is not thread-safe
    template<typename = void>
    TerminalInfo& detect();

    // Common control sequences
    // The following methods return IO manipulators for std::ostream

    auto title(string_view title) const {
         return detail::make_ansi_manip_wrapper(mode, ansi::title(title));
    }

    auto reset() const {
        return detail::make_ansi_manip_wrapper(mode, ansi::reset);
    }

    auto bold() const {
        return detail::make_ansi_manip_wrapper(mode, ansi::bold);
    }

    auto clear() const {
        return detail::make_ansi_manip_wrapper(mode, ansi::clear);
    }

    auto clear_line() const {
        return detail::make_ansi_manip_wrapper(mode, ansi::clear_line);
    }

    auto line_start() const {
        return detail::make_ansi_manip_wrapper(mode, ansi::line_start);
    }

    auto foreground(ansi::Color c) const {
        return detail::make_ansi_manip_wrapper(
            supported(TerminalMode::Ansi) ? TerminalMode::Ansi : TerminalMode::None,
            ansi::foreground(c));
    }

    auto background(ansi::Color c) const {
        return detail::make_ansi_manip_wrapper(
            supported(TerminalMode::Ansi) ? TerminalMode::Ansi : TerminalMode::None,
            ansi::background(c));
    }

    detail::foreground_setter foreground(Color c) const {
        return { mode, c };
    }

    detail::background_setter background(Color c) const {
        return { mode, c };
    }

    auto move_to(Point loc) const {
        return detail::make_ansi_manip_wrapper(mode, ansi::move_to(loc));
    }

    auto move_up(unsigned count = 1) const {
        return detail::make_ansi_manip_wrapper(mode, ansi::move_up(count));
    }

    auto move_down(unsigned count = 1) const {
        return detail::make_ansi_manip_wrapper(mode, ansi::move_down(count));
    }

    auto move_forward(unsigned count = 1) const {
        return detail::make_ansi_manip_wrapper(mode, ansi::move_forward(count));
    }

    auto move_backward(unsigned count = 1) const {
        return detail::make_ansi_manip_wrapper(mode, ansi::move_backward(count));
    }

    TerminalMode mode;
    Color foreground_color;
    Color background_color;

private:
    template<typename = void>
    std::string query(string_view query, string_view terminator);

    Terminal term;
};


namespace detail
{
    struct tcsetattr_guard
    {
        tcsetattr_guard(Terminal term, struct termios old, struct termios new_)
            : term(term), old(old), new_(new_)
            {}

        ~tcsetattr_guard() {
            if (ok)
                tcsetattr(term, TCSANOW, &old);
        }

        bool set() {
            return (ok = !tcsetattr(term, TCSANOW, &new_));
        }

        bool ok = false;
        Terminal term;
        struct termios old, new_;
    };
} /* namespace detail */


template<typename>
TerminalInfo& TerminalInfo::detect() {
    if (!is_terminal())
        return *this;

    string_view name, colorterm, vte_version;

    auto tmp = std::getenv("TERM");
    if (tmp) name = tmp;

    tmp = std::getenv("COLORTERM");
    if (tmp) colorterm = tmp;

    tmp = std::getenv("VTE_VERSION");
    if (tmp) vte_version = tmp;

    bool xterm_like = detail::contains(name, "xterm");

    bool has_truecolor =
        !vte_version.empty()
            ? (vte_version[0] > '3' || (vte_version[0] == '3' && vte_version[1] >= '6')) // VTE >= 0.36 supports true color
            : detail::contains(colorterm, "truecolor") ||
              detail::contains(name, "cygwin") ||
              detail::contains(colorterm, "cygwin") ||
              detail::contains(name, "konsole") ||
              detail::contains(colorterm, "konsole");

    bool has_256color = has_truecolor ||
                        detail::contains(name, "256") ||
                        !colorterm.empty();

    bool has_ansi = has_256color ||
                    xterm_like ||
                    detail::contains(name, "screen") ||
                    detail::contains(name, "vt100") ||
                    detail::contains(name, "color") ||
                    detail::contains(name, "ansi") ||
                    detail::contains(name, "cygwin") ||
                    detail::contains(name, "linux");

    if (mode == TerminalMode::None)
        mode = has_truecolor ? TerminalMode::Iso24bit
                             : has_256color ? TerminalMode::Ansi256
                                            : has_ansi ? TerminalMode::Ansi
                                                       : TerminalMode::None;

    if (xterm_like && foreground_color == Color(0.9f, 0.9f, 0.9f, 1)) {
        auto response = query("\x1b]10;?\x1b\\", "\a\\");
        if (!response.empty()) {
            auto pos = response.find("rgb:");
            if (pos != std::string::npos) {
                pos += 4;
                Color32 c = { 255, 255, 255, 255 };
                if (sscanf(response.c_str() + pos, "%2hhx%*2x/%2hhx%*2x/%2hhx%*2x", &c.r, &c.g, &c.b) == 3)
                    foreground_color = c;
            }
        }
    }

    if (xterm_like && background_color == Color(0, 0, 0, 1)) {
        auto response = query("\x1b]11;?\x1b\\", "\a\\");
        if (!response.empty()) {
            auto pos = response.find("rgb:");
            if (pos != std::string::npos) {
                pos += 4;
                Color32 c = { 0, 0, 0, 255 };
                if (sscanf(response.c_str() + pos, "%2hhx%*2x/%2hhx%*2x/%2hhx%*2x", &c.r, &c.g, &c.b) == 3)
                    background_color = c;
            }
        }
    }

    return *this;
}

template<typename>
std::string TerminalInfo::query(string_view query, string_view terminator) {
    struct termios oldAttrs = {};
    if (tcgetattr(STDOUT_FILENO, &oldAttrs))
        return std::string();

    struct termios newAttrs = oldAttrs;
    newAttrs.c_lflag &= ~(ECHO | ICANON);
    newAttrs.c_cc[VMIN] = 0;
    newAttrs.c_cc[VTIME] = 0;

    detail::tcsetattr_guard guard(term, oldAttrs, newAttrs);
    if (!guard.set())
        return std::string();

    if (tcdrain(STDOUT_FILENO))
        return std::string();

    if (tcflush(STDOUT_FILENO, TCIFLUSH))
        return std::string();

    if (std::size_t(write(STDOUT_FILENO, query.data(), query.size())) != query.size())
        return std::string();

    // FIXME: This won't be enough for remote terminals (e.g. SSH)
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    int available = 0;
    if (ioctl(STDOUT_FILENO, FIONREAD, &available))
        return std::string();

    std::string result;

    while (available) {
        result.reserve(result.size() + available);

        for (; available > 0; --available) {
            char ch = '\0';
            if (read(STDOUT_FILENO, &ch, 1) != 1)
                return std::string();

            if (!result.empty() || ch == '\x1b') {
                result.append(1, ch);
                if (terminator.find(ch) != string_view::npos)
                    return result;
            }
        }

        // If we found an escape character but no terminator, continue reading
        if (!result.empty())
            if (ioctl(STDOUT_FILENO, FIONREAD, &available))
                return std::string();
    }

    return std::string();
}

#else
#error "Non-POSIX systems are not supported"
#endif

} /* namespace plot */
