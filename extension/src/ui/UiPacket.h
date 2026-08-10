#pragma once

#include "../input/InputData.h"
#include "../state/StateTypes.h"
#include "../feedback/FeedbackData.h"

struct UiPacket {
    // State machine
    ControlState state = ControlState::AUTO_MODE;
    PanelMode selectedMode = PanelMode::AUTO;

    // Physical input
    bool forwardPressed = false;
    bool backwardPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
    bool speedUpPressed = false;
    bool speedDownPressed = false;

    // CAN feedback
    uint8_t activationStatus = 0;
    uint8_t currentGear = 0;
    uint8_t outputFeedback = 0;

    uint16_t socRaw = 0;
    float socPercent = 0.0f;

    int32_t speedLeftFeedback = 0;
    int32_t speedRightFeedback = 0;
};