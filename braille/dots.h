#pragma once

#include "tui/color.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

namespace braille {

// 4 cols x 2 rows
class Dots {
public:
    void Set(int y, int x, bool state) {
        assert(y >= 0 && y < 4);
        assert(x >= 0 && x < 2);

        uint8_t bit;
        if (y < 3) {
            bit = (1u << (3 * x + y));
        } else {
            bit = (1u << (6 + x));
        }
        if (state == true) {
            state_ |= bit;
        } else {
            state_ &= ~bit;
        }
    }
    
    uint32_t Get() const {
        return L'\u2800' + state_;
    }

private:
    uint8_t state_ = 0;
};

}  // namespace braille

