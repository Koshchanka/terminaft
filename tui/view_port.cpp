#include "view_port.h"

namespace tui {

namespace {

}  // namespace

void ViewPort::Render(bool do_clear) {
    caret_ = buff_;

    SetBackgroundColor(Color::kDefault, true);
    SetForegroundColor(Color::kDefault, true);
    if (do_clear) {
        ClearScreen();
    } else {
        ResetCursor();
    }

    auto dims = GetDimensions();
    for (int i = 0; i < chars_.size() && i < dims.y; ++i) {
        for (int j = 0; j < chars_[0].size() && j < dims.x; ++j) {
            auto& ch = chars_[i][j];
            if (ch.unicode == 0) {
                ch.unicode = ' ';
            }
            Put(chars_[i][j]);
        }
        if (i + 1 < chars_.size() && i + 1 < dims.y) {
            Put('\n');
        }
    }

    SetBackgroundColor(Color::kDefault, true);
    SetForegroundColor(Color::kDefault, true);
    auto written = 0;
    auto to_write = caret_ - buff_;
    while (written < to_write) {
        auto res = write(STDOUT_FILENO, buff_ + written, to_write - written);
        assert(res != -1 && "couldn't write to STDOUT_FILENO");
        written += res;
    }
}

}  // namespace tui
