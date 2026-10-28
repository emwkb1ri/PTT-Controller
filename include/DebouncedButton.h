#pragma once

#include <stdint.h>

class DebouncedButton {
public:
    explicit DebouncedButton(uint32_t intervalMs = 25)
        : intervalMs_(intervalMs) {}

    void begin(bool pressed, uint32_t now) {
        rawPressed_ = pressed;
        stablePressed_ = pressed;
        changedAt_ = now;
    }

    bool update(bool pressed, uint32_t now) {
        if (pressed != rawPressed_) {
            rawPressed_ = pressed;
            changedAt_ = now;
        }

        if (rawPressed_ != stablePressed_ &&
            static_cast<uint32_t>(now - changedAt_) >= intervalMs_) {
            stablePressed_ = rawPressed_;
            return true;
        }

        return false;
    }

private:
    const uint32_t intervalMs_;
    uint32_t changedAt_ = 0;
    bool rawPressed_ = false;
    bool stablePressed_ = false;
};