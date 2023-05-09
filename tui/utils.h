#pragma once

#include "color.h"

namespace tui::utils {

struct Dims {
    int x;
    int y;
};

Dims GetScreenDimensions();

void ClearScreen();

void SetForegroundColor(Color);

void SetBackgroundColor(Color);

}  // namespace tui

