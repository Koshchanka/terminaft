#include "utils.h"

#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <string>

namespace tui::utils {

namespace {

void PutEsc(int code, char ch) {
    char buff[16];
    sprintf(buff, "\033[%i%c", code, ch);
    write(STDOUT_FILENO, buff, strlen(buff));
}

void PutDoubleEsc(int code1, int code2, char ch) {
    char buff[16];
    sprintf(buff, "\033[%i;%i%c", code1, code2, ch);
    write(STDOUT_FILENO, buff, strlen(buff));
}

void PutTrippleEsc(int code1, int code2, int code3, char ch) {
    char buff[16];
    sprintf(buff, "\033[%i;%i;%i%c", code1, code2, code3, ch);
    write(STDOUT_FILENO, buff, strlen(buff));
}

}  // namespace

Dims GetScreenDimensions() {
    winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return { .x = w.ws_col, .y = w.ws_row };
}

void ResetCursor() {
    PutDoubleEsc(0, 0, 'f');
}

void ClearScreen() {
    PutEsc(2, 'J');
    ResetCursor();
}

void SetForegroundColor(Color color) {
    if (color == Color::kDefault) {
        PutEsc(static_cast<int>(39), 'm');
        return;
    }
    PutTrippleEsc(38, 5, static_cast<int>(color), 'm');
}

void SetBackgroundColor(Color color) {
    if (color == Color::kDefault) {
        PutEsc(static_cast<int>(49), 'm');
        return;
    }
    PutTrippleEsc(48, 5, static_cast<int>(color), 'm');
}

}  // namespace tui

