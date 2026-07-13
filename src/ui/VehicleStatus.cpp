#include "VehicleStatus.h"

// ==========================
// Global vehicle status object
// ==========================

VehicleStatus vehicleStatus = {
    CONTROL_SOURCE_ERROR,  // controlSource

    false,                 // batteryValid
    0.0f,                  // battery

    false,                 // driveModeValid, false = Null
    false,                 // manualMode, false = Automatic, true = Manual

    false,                 // steeringValid
    DIRECTION_NULL,        // steering

    false,                 // speedValid
    0,                     // leftSpeed
    0,                     // rightSpeed

    false                  // estopCommandSent
};

// ==========================
// Helper functions
// ==========================

static int32_t abs_i32(int32_t value)
{
    return value < 0 ? -value : value;
}

// ==========================
// Reset vehicle status
// ==========================

void reset_vehicle_status()
{
    vehicleStatus.controlSource =
        CONTROL_SOURCE_ERROR;

    vehicleStatus.batteryValid = false;
    vehicleStatus.battery = 0.0f;

    vehicleStatus.driveModeValid = false;
    vehicleStatus.manualMode = false;

    vehicleStatus.steeringValid = false;
    vehicleStatus.steering = DIRECTION_NULL;

    vehicleStatus.speedValid = false;
    vehicleStatus.leftSpeed = 0;
    vehicleStatus.rightSpeed = 0;

    vehicleStatus.estopCommandSent = false;
}

// ==========================
// Update vehicle mode from state machine
// manualMode = true  -> Manual
// manualMode = false -> Automatic
//
// Control source is handled separately:
// - CanProtocol.cpp sets REMOTE
// - ControlStateMachine.cpp sets PANEL
// - CAN timeout sets ERROR
// ==========================

void update_vehicle_mode(bool manualMode)
{
    vehicleStatus.manualMode = manualMode;
    vehicleStatus.driveModeValid = true;
}

// ==========================
// Steering evaluation
// ==========================

void update_steering(int32_t leftRaw, int32_t rightRaw)
{
    int32_t leftLogical = -leftRaw;
    int32_t rightLogical = rightRaw;

    const int32_t ZERO_THRESHOLD = 10;
    const int32_t TURN_THRESHOLD = 20;

    if (abs_i32(leftLogical) <= ZERO_THRESHOLD) {
        leftLogical = 0;
    }

    if (abs_i32(rightLogical) <= ZERO_THRESHOLD) {
        rightLogical = 0;
    }

    if (leftLogical == 0 && rightLogical == 0) {
        vehicleStatus.steering = DIRECTION_NULL;
        vehicleStatus.steeringValid = false;
        return;
    }

    int32_t difference = rightLogical - leftLogical;

    if (abs_i32(difference) <= TURN_THRESHOLD) {
        int32_t average =
            (leftLogical + rightLogical) / 2;

        if (average > 0) {
            vehicleStatus.steering =
                DIRECTION_FORWARD;

            vehicleStatus.steeringValid = true;
        }
        else if (average < 0) {
            vehicleStatus.steering =
                DIRECTION_BACKWARD;

            vehicleStatus.steeringValid = true;
        }
        else {
            vehicleStatus.steering =
                DIRECTION_NULL;

            vehicleStatus.steeringValid = false;
        }

        return;
    }

    if (difference > TURN_THRESHOLD) {
        vehicleStatus.steering =
            DIRECTION_LEFT;

        vehicleStatus.steeringValid = true;

        return;
    }

    if (difference < -TURN_THRESHOLD) {
        vehicleStatus.steering =
            DIRECTION_RIGHT;

        vehicleStatus.steeringValid = true;

        return;
    }

    vehicleStatus.steering =
        DIRECTION_NULL;

    vehicleStatus.steeringValid = false;
}