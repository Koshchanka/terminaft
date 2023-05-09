#pragma once

#include "char.h"

#include <vector>

namespace tui {

class Object {
public:
    using Rectangle = std::vector<std::vector<Char>>;

    Object(int h, int w) : h_(h), w_(w) {
    }

    int Width() const {
        return w_;
    }

    int Height() const {
        return h_;
    }

    virtual Rectangle BuildChars() const = 0;

protected:
    int h_;
    int w_;
};

}  // namespace tui

