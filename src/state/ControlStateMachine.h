#pragma once

#include "StateTypes.h"
#include "../input/InputData.h"

// ================================================================
// Control state machine
// ================================================================

// Initialize state machine
void control_state_init();

// Update state machine using current physical input
void control_state_update(
    const PhysicalInput& input
);

// Get current state
ControlState get_control_state();