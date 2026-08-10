#include "UiDataPacker.h"

// ================================================================
// Latest UI packet
// ================================================================
//
// This variable stores the newest data packet for the UI system.
// It is static, so it can only be accessed inside this .cpp file.
// Other files must use getLatestUiPacket() or copyLatestUiPacket().
// ================================================================

static UiPacket latestPacket;

// ================================================================
// Update UI packet
// ================================================================
//
// This function combines:
//   1. Physical input state
//   2. Current control state machine state
//   3. Feedback received from the main CAN system
//
// into one UiPacket.
// ================================================================

void updateUiPacket(
    const PhysicalInput& input,
    ControlState state,
    const MainSystemFeedback& feedback
) {
    // ------------------------------------------------------------
    // State machine information
    // ------------------------------------------------------------

    latestPacket.state = state;
    latestPacket.selectedMode = input.selectedMode;

    // ------------------------------------------------------------
    // Physical input information
    // ------------------------------------------------------------

    latestPacket.forwardPressed = input.forward;
    latestPacket.backwardPressed = input.backward;
    latestPacket.leftPressed = input.left;
    latestPacket.rightPressed = input.right;

    latestPacket.speedUpPressed = input.speedUp;
    latestPacket.speedDownPressed = input.speedDown;

    // ------------------------------------------------------------
    // CAN feedback information
    // ------------------------------------------------------------

    latestPacket.activationStatus = feedback.activationStatus;
    latestPacket.currentGear = feedback.currentGear;
    latestPacket.outputFeedback = feedback.outputFeedback;

    latestPacket.socRaw = feedback.socRaw;
    latestPacket.socPercent = feedback.socPercent;

    latestPacket.speedLeftFeedback = feedback.speedLeft;
    latestPacket.speedRightFeedback = feedback.speedRight;
}

// ================================================================
// Direct UI access
// ================================================================
//
// The UI system can call this function to get the newest packet.
// It returns a const reference, so the UI can read it but should not
// modify it directly.
// ================================================================

const UiPacket& getLatestUiPacket() {
    return latestPacket;
}

// ================================================================
// Safe copy access
// ================================================================
//
// This returns a copy of the latest packet.
// It is safer if the UI and communication logic later run in
// different tasks.
// ================================================================

UiPacket copyLatestUiPacket() {
    return latestPacket;
}