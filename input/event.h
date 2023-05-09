#pragma once

namespace input {

enum class Key {
    W,
    A,
    S,
    D,
    Up,
    Down,
    Left,
    Right,
    U,
    J,
    MouseLeft,
    MouseRight
};

enum class Action {
    kKeyboard,
    kMouse,
};

struct Event {
    Action action;
    Key key;
};

}  // namespace input

