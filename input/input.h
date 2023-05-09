#pragma once

#include "event.h"

#include <cassert>
#include <fcntl.h>
#include <functional>
#include <optional>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace input {

class EventPoller {
public:
    EventPoller() {
        struct termios terminal;
        tcgetattr(STDIN_FILENO, &terminal);
        on_destroy_ = [=] { tcsetattr(STDIN_FILENO, TCSANOW, &terminal); };
        terminal.c_lflag &= ~ICANON;
        terminal.c_lflag &= ~ECHO;
        terminal.c_cc[VMIN] = 0;
        terminal.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
    }

    ~EventPoller() {
        on_destroy_();
    }

    std::optional<Event> Poll() {
        auto next = TryPeek();
        if (!next) {
            return std::nullopt;
        }
        if (next == '\033') {
            if (!(next = TryPeek())) {
                assert(false);
            }
            if (!(next = TryPeek())) {
                assert(false);
            }
            switch (next) {
                case 'A': {
                    return Event{ Action::kKeyboard, Key::Up };
                }
                case 'B': {
                    return Event{ Action::kKeyboard, Key::Down };
                }
                case 'C': {
                    return Event{ Action::kKeyboard, Key::Left };
                }
                case 'D': {
                    return Event{ Action::kKeyboard, Key::Right };
                }
                default: {
                    return std::nullopt;
                }
            }
        }
        switch (next) {
            case 'w':
            case 'W': {
                    return Event{ Action::kKeyboard, Key::W };
            }
            case 'a':
            case 'A': {
                    return Event{ Action::kKeyboard, Key::A };
            }
            case 's':
            case 'S': {
                    return Event{ Action::kKeyboard, Key::S };
            }
            case 'd':
            case 'D': {
                    return Event{ Action::kKeyboard, Key::D };
            }
            default: {
                return std::nullopt;
            }
        }
    }

private:
    char TryPeek() {
        if (start_ != end_) {
            auto result = buff_[start_];
            ++start_;
            return result;
        }

        struct pollfd pfd;

        pfd.fd = STDIN_FILENO;
        pfd.events = POLLIN;

        if (!poll(&pfd, 1, 5) || !(pfd.revents & POLLIN)) {
            return 0;
        }

        auto res = read(STDIN_FILENO, buff_, kBuffLen);
        assert(res != -1);
        end_ = res;
        start_ = 0;

        if (res == 0) {
            assert(false);
            return 0;
        }

        return buff_[start_++];
    }

    void Consume() {
        ++start_;
    }

    void PutBack() {
        assert(start_ != 0);
        --start_;
    }

    static constexpr int kBuffLen = 64;
    char buff_[64];

    int start_ = 0;
    int end_ = 0;

    std::function<void()> on_destroy_;
};

}  // namespace input

