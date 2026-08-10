#pragma once

#include <Arduino.h>

#include "CanFrame.h"
#include "../feedback/FeedbackData.h"

// ================================================================
// Command data for manual crawler control
// ================================================================

enum class MovementCommand {
    NEUTRAL,
    FORWARD,
    BACKWARD,
    LEFT_TURN,
    RIGHT_TURN
};

struct ManualControlCommand {
    MovementCommand movement = MovementCommand::NEUTRAL;
    uint16_t speedMagnitude = 0;   // 0 ... 1000
    uint8_t gear = 1;              // usually 1 in MANUAL_ACTIVE
};

// ================================================================
// RX parsing
// ================================================================

bool parseStatus215(const CanFrame& frame, MainSystemFeedback& feedback);
bool parseSpeed315(const CanFrame& frame, MainSystemFeedback& feedback);

// ================================================================
// TX frame builders
// ================================================================

CanFrame buildExternalControlRequest195();
CanFrame buildAuthReply195(uint8_t randomNumber, uint8_t shiftValue);
CanFrame buildDeactivate195();

CanFrame buildNeutral295();
CanFrame buildManualControl295(const ManualControlCommand& command);