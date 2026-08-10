#pragma once

#include <Arduino.h>

struct MainSystemFeedback {
    uint8_t activationStatus = 0;
    uint8_t randomNumber = 0;
    uint8_t shiftValue = 0;

    uint8_t outputFeedback = 0;
    uint8_t currentGear = 0;

    uint16_t socRaw = 0;
    float socPercent = 0.0f;

    int32_t speedLeft = 0;
    int32_t speedRight = 0;
};