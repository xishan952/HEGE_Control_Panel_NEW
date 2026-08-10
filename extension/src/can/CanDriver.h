#pragma once

#include <Arduino.h>
#include "CanFrame.h"

// ==========================
// CAN driver state
// ==========================

enum class CanDriverState {
    NOT_CONFIGURED,
    READY,
    ERROR
};

// ==========================
// CAN driver interface
// ==========================

bool can_driver_init();

bool can_driver_is_ready();

CanDriverState can_driver_get_state();

bool can_driver_send(const CanFrame& frame);

bool can_driver_receive(CanFrame& frame);