#pragma once

#include "char.h"
#include "object.h"

#include <cassert>
#include <string>

namespace tui {

class Border : public Object {
public:
    Border(int h, int w, Color color)
        : Object(h, w)
        , color_(color)
    {
    }

    Rectangle BuildChars() const override {
        Rectangle result(Height(), std::vector<Char>(Width()));
        for (int i = 0; i < Height(); ++i) {
            result[i][0] = result[i][Width() - 1] = { .unicode = '|', .fg = color_ };
        }
        for (int j = 0; j < Width(); ++j) {
            result[0][j] = result[Height() - 1][j]  = { .unicode = '-', .fg = color_ };
        }
        result[0][0] = result[Height() - 1][0] = result[0][Width() - 1] = result[Height() - 1][Width() - 1] = { .unicode = '+', .fg = color_ };
        return result;
    }

private:
    Color color_;
};

class Textbox : public Object {
public:
    Textbox(std::string text, tui::Color color)
        : Object(1, text.size())
        , text_(std::move(text))
        , color_(color)
    {
    }

    Rectangle BuildChars() const override {
        assert(Height() == 1 && Width() == text_.size());
        Rectangle result(Height(), std::vector<Char>(Width()));
        for (int i = 0; i < Width(); ++i) {
            result[0][i] = { .unicode = static_cast<uint32_t>(text_[i]), .fg = color_ };
        }
        return result;
    }

private:
    std::string text_;
    tui::Color color_;
};

}  // namespace tui
