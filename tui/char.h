#pragma once

#include "color.h"

#include <cstdint>

namespace tui {

struct Char {
    uint32_t unicode = 0;
    Color fg = Color::kDefault;
    Color bg = Color::kDefault;
};

}  // namepsace tui

