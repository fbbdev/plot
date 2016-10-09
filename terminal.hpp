#pragma once

namespace plot
{

enum class TerminalColor {
    None,       // Do not output control sequences
    Minimal,    // Reset attributes, use bold
    Ansi,       // Use ANSI codes for standard 8-color palette
    Ansi256,    // Use ANSI sequences for xterm 256 color mode
    Iso24bit    // Use ISO-8613-3 sequences for 24-bit true-color mode
};

enum class TerminalOp {
    Over,       // Paint source over destination, mix color block by block
    ClipDst,    // Erase destination where source is not empty
    ClipSrc     // Ignore source where destination is not empty
};

}
