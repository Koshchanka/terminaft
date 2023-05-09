#include "char.h"
#include "object.h"
#include "utf8.h"
#include "utils.h"

#include <cassert>
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <unistd.h>

namespace tui {

class ViewPort {
public:
    ViewPort(int h, int w)
        : w_(w)
        , h_(h)
        , chars_(h_, std::vector<Char>(w_))
    {
        buff_ = (char*) malloc(20 * w_ * h_);
        assert(w_ > 0 && h_ > 0);
    }

    ~ViewPort() {
        free(buff_);
    }

    void Clear() {
        caret_ = buff_;
        ClearScreen();
        write(STDOUT_FILENO, buff_, caret_ - buff_);
    }

    int Width() const {
        return w_;
    }

    int Height() const {
        return h_;
    }

    void SetChar(int y, int x, const Char& ch) {
        chars_.at(y).at(x) = ch;
    }

    void SetChar(int y, int x, uint32_t unicode, Color fg = Color::kDefault, Color bg = Color::kDefault) {
        SetChar(y, x, {.unicode = unicode, .fg = fg, .bg = bg});
    }

    void PlaceObject(int y, int x, const Object& object) {
        SetRectangle(y, x, object.BuildChars());
    }

    void SetRectangle(int y, int x, const std::vector<std::vector<tui::Char>>& rect) {
        for (int i = std::max(0, -y); i < rect.size() && y + i < h_; ++i) {
            for (int j = std::max(0, -x); j < rect[0].size() && x + j < w_; ++j) {
                if (rect[i][j].unicode == 0) {
                    continue;
                }
                SetChar(y + i, x + j, rect[i][j]);
            }
        } 
    }

    void Render(bool do_clear = false);

private:
    utils::Dims GetDimensions() const {
        return utils::GetScreenDimensions();
    }

    int PutEsc(int code, char ch) {
        return sprintf(caret_, "\033[%i%c", code, ch);
    }

    int PutDoubleEsc(int code1, int code2, char ch) {
        return sprintf(caret_, "\033[%i;%i%c", code1, code2, ch);
    }

    int PutTrippleEsc(int code1, int code2, int code3, char ch) {
        return sprintf(caret_, "\033[%i;%i;%i%c", code1, code2, code3, ch);
    }


    void ResetCursor() {
        caret_ += PutDoubleEsc(0, 0, 'f');
    }

    void ClearScreen() {
        caret_ += PutEsc(2, 'J');
        ResetCursor();
    }

    void SetForegroundColor(Color color, bool force = false) {
        if (!force && fg_ == color) {
            return;
        }
        fg_ = color;
        if (color == Color::kDefault) {
            caret_ += PutEsc(static_cast<int>(39), 'm');
            return;
        }
        caret_ += PutTrippleEsc(38, 5, static_cast<int>(color), 'm');
    }

    void SetBackgroundColor(Color color, bool force = false) {
        if (!force && bg_ == color) {
            return;
        }
        bg_ = color;
        if (color == Color::kDefault) {
            caret_ += PutEsc(static_cast<int>(49), 'm');
            return;
        }
        caret_ += PutTrippleEsc(48, 5, static_cast<int>(color), 'm');
    }

    void Put(uint32_t utf) {
        caret_ += Utf8Encode(caret_, utf);
    }

    void Put(const Char& ch) {
        SetBackgroundColor(ch.bg);
        SetForegroundColor(ch.fg);
        Put(ch.unicode);
    }

    int w_;
    int h_;
    std::vector<std::vector<Char>> chars_;
    char* buff_;
    char* caret_;

    Color fg_;
    Color bg_;
};

}  // namespace tui
