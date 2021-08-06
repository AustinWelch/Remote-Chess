#pragma once

namespace RemoteChess {
    struct ButtonState {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        bool center = false;
    };

    struct ButtonInterface {
        ButtonInterface();

        ButtonState GetCurrentButtonState() const;
    };
}
