#pragma once

#include <Arduino.h>

// ==========================
// Vehicle direction
// ==========================

enum VehicleDirection {
    DIRECTION_NULL,
    DIRECTION_FORWARD,
    DIRECTION_BACKWARD,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
};

// ==========================
// Current control source
// ==========================

enum ControlSource {
    CONTROL_SOURCE_ERROR,
    CONTROL_SOURCE_REMOTE,
    CONTROL_SOURCE_PANEL
};

// ==========================
// Vehicle status data structure
// CAN protocol and control logic update this structure.
// UI only reads from it.
// ==========================

struct VehicleStatus {
    ControlSource controlSource;

    bool batteryValid;
    float battery;              // percent, SOC raw * 0.1

    bool driveModeValid;
    bool manualMode;            // true = Manual, false = Automatic

    bool steeringValid;
    VehicleDirection steering;

    bool speedValid;
    int leftSpeed;              // scaled value: 0 ... 1000
    int rightSpeed;             // scaled value: 0 ... 1000

    bool estopCommandSent;      // true = external E-Stop input active / 0x150 sent
};

// ==========================
// Global vehicle status object
// It is defined in VehicleStatus.cpp
// ==========================

extern VehicleStatus vehicleStatus;

// ==========================
// Vehicle status functions
// ==========================

void reset_vehicle_status();

void update_steering(
    int32_t leftRaw,
    int32_t rightRaw
);

// Called when HEGE authentication / control mode changes
void update_vehicle_mode(bool manualMode);