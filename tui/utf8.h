#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace tui {

inline size_t Utf8Encode(char* out, uint32_t unicode) {
    if (unicode <= 0x7F) {
        out[0] = (char) unicode;
        out[1] = 0;
        return 1;
    } else if (unicode <= 0x07FF) {
        out[0] = ((unicode >> 6) & 0x1F) | 0xC0;
        out[1] = (unicode & 0x3F) | 0x80;
        out[2] = 0;
        return 2;
    } else if (unicode <= 0xFFFF) {
        out[0] = ((unicode >> 12) & 0x0F) | 0xE0;
        out[1] = ((unicode >> 6) & 0x3F) | 0x80;
        out[2] = (unicode & 0x3F) | 0x80;
        out[3] = 0;
        return 3;
    } else if (unicode <= 0x10FFFF) {
        out[0] = ((unicode >> 18) & 0x07) | 0xF0;
        out[1] = ((unicode >> 12) & 0x3F) | 0x80;
        out[2] = ((unicode >> 6) & 0x3F) | 0x80;
        out[3] = (unicode & 0x3F) | 0x80;
        out[4] = 0;
        return 4;
    } else { 
        assert(false && "invalid Unicode code-point");
    }
}

}  // namespace tui

