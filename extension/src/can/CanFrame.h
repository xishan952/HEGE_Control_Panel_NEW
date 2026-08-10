#pragma once

#include <Arduino.h>

// ================================================================
// Generic CAN frame used inside our software
// ================================================================

struct CanFrame {
    uint32_t id = 0;
    uint8_t dlc = 8;
    uint8_t data[8] = {0};
};