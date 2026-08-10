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
// Vehicle status data structure
// Main program updates this structure.
// UI only reads from it.
// ==========================

struct VehicleStatus {
    bool canConnected;          // true = HEGE authenticated / takes control

    bool batteryValid;
    float battery;              // percent, SOC raw * 0.1

    bool driveModeValid;        // false = Null, true = CAN activity detected
    bool manualMode;            // true = Manual, false = Automatic

    bool steeringValid;
    VehicleDirection steering;

    bool speedValid;
    int leftSpeed;              // scaled value: 0 ... 1000
    int rightSpeed;             // scaled value: 0 ... 1000

    bool estopValid;
    bool emergencyReleased;     // true = Released, false = Pressed
};

// ==========================
// Global vehicle status object
// It is defined in VehicleStatus.cpp
// ==========================

extern VehicleStatus vehicleStatus;

// ==========================
// Function declarations
// ==========================

void reset_vehicle_status();

void parse_can_message(uint32_t id, const uint8_t* data, uint8_t dlc);

void update_steering(int32_t leftRaw, int32_t rightRaw);

// Called when HEGE authentication / control mode changes
void update_vehicle_mode(bool manualMode);

void update_vehicle_status_timeout();