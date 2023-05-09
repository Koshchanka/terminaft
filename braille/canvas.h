#pragma once

#include "dots.h"
#include "tui/char.h"
#include "tui/object.h"

#include <vector>

namespace braille {

class Canvas : public tui::Object {
public:
    Canvas(int h, int w)
        : Object(h / 4, w / 2)
    {
        assert(h % 4 == 0);
        assert(w % 2 == 0);

        canvas_.assign(h / 4, std::vector<Dots>(w / 2));
        colors_.assign(h / 4, std::vector<tui::Color>(w / 2, tui::Color::kWhite));
    }

    void Set(int y, int x, bool state) {
        canvas_.at(y / 4).at(x / 2).Set(y % 4, x % 2, state);
    }

    void SetColor(int block_y, int block_x, tui::Color color) {
        colors_.at(block_y).at(block_x) = color;
    }

    tui::Object::Rectangle BuildChars() const {
        tui::Object::Rectangle result(
            canvas_.size(), std::vector<tui::Char>(canvas_.at(0).size()));
        for (size_t i = 0; i < canvas_.size(); ++i) {
            for (size_t j = 0; j < canvas_[0].size(); ++j) {
                result[i][j] = { .unicode = canvas_[i][j].Get(), .fg = colors_[i][j] };
            }
        }
        return result;
    }

private:
    template<typename T>
    struct Coord {
        T y;
        T x;
    };

    std::vector<std::vector<Dots>> canvas_;
    std::vector<std::vector<tui::Color>>  colors_;
};

}  // namespace braille

