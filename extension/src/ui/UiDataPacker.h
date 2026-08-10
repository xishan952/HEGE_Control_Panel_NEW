#pragma once

#include "UiPacket.h"
#include "../input/InputData.h"
#include "../state/StateTypes.h"
#include "../feedback/FeedbackData.h"

// ================================================================
// UI Data Packer
// ================================================================
//
// This module combines:
//   1. Physical input
//   2. Control state machine state
//   3. CAN feedback from main system
//
// into one UiPacket.
//
// The UI system can directly call getLatestUiPacket().
// ================================================================

void updateUiPacket(
    const PhysicalInput& input,
    ControlState state,
    const MainSystemFeedback& feedback
);

const UiPacket& getLatestUiPacket();

UiPacket copyLatestUiPacket();