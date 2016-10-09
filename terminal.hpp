#pragma once

#include "point.hpp"

#include <cstdlib>
#include <string>

#if defined(__unix__) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#define PLOT_PLATFORM_POSIX
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace plot
{

enum class TerminalColor {
    None,       // Do not output control sequences
    Minimal,    // Reset attributes, use bold
    Ansi,       // Use ANSI codes for standard 8-color palette
    Ansi256,    // Use ANSI sequences for xterm 256 color mode
    Iso24bit,   // Use ISO-8613-3 sequences for 24-bit true-color mode
    // Windows     // Windows console API
};

enum class TerminalOp {
    Over,       // Paint source over destination, mix character colors
    ClipDst,    // Erase destination where source is not empty
    ClipSrc     // Ignore source where destination is not empty
};

#ifdef PLOT_PLATFORM_POSIX

using Terminal = int;

class TerminalInfo {
public:
    TerminalInfo(Terminal term = STDOUT_FILENO)
        : term(term)
    {
        auto n = std::getenv("TERM");
        if (n)
            name = n;
    }

    bool is_terminal() const {
        return isatty(term);
    }

    Size size() const {
        struct winsize ws = {};

        if (ioctl(term, TIOCGWINSZ, &ws)) {
            return {};
        }

        return { ws.ws_col, ws.ws_row };
    }

    TerminalColor color_mode() const {
        // FIXME: check name to detect 256-color and truecolor support
        return is_terminal() ? TerminalColor::Ansi : TerminalColor::None;
    }

    bool supported(TerminalColor mode) {
        return mode == TerminalColor::None || is_terminal();
    }

private:
    Terminal term;
    std::string name;
};

#else

using Terminal = void*;

class TerminalInfo {
public:
    constexpr TerminalInfo(Terminal term = nullptr)
        : term(term)
        {}

private:
    Terminal term;
};

#endif

} /* namespace plot */
