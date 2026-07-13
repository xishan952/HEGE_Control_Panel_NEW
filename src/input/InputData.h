#pragma once

#include <Arduino.h>

enum class PanelMode {
    AUTO,
    MANUAL
};

struct PhysicalInput {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;

    bool speedUp = false;
    bool speedDown = false;

    bool estop = false;

    PanelMode selectedMode = PanelMode::AUTO;
};