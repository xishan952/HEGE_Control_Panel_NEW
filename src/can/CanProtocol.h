#pragma once

#include <Arduino.h>

#include "CanFrame.h"

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
// RX status
// ================================================================

// Set to true when CAN ID 0x140 is received
// with Byte 0 == 0x01.
extern bool controlAnswerOk;

// ================================================================
// RX parsing
// ================================================================

void parse_can_message(
    uint32_t id,
    const uint8_t* data,
    uint8_t dlc
);

void update_vehicle_status_timeout();

// ================================================================
// TX frame builders
// ================================================================

// 0x150: request manual control
CanFrame buildControlRequest150();

// 0x150: send emergency stop command
CanFrame buildEstop150();

// 0x330: manual crawler speed command
CanFrame buildSpeedCommand330(
    const ManualControlCommand& command
);