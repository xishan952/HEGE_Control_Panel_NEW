#include "VehicleStatus.h"

// ==========================
// Global vehicle status object
// ==========================

static unsigned long lastBatteryMs = 0;
static unsigned long lastSpeedMs = 0;
static unsigned long lastEstopMs = 0;

static const unsigned long BATTERY_TIMEOUT_MS = 1000;
static const unsigned long SPEED_TIMEOUT_MS = 500;
static const unsigned long ESTOP_TIMEOUT_MS = 1000;

VehicleStatus vehicleStatus = {
    false,              // canConnected = HEGE authenticated / takes control

    false,              // batteryValid
    0.0f,               // battery

    false,              // driveModeValid, false = Null
    false,              // manualMode, false = Automatic, true = Manual

    false,              // steeringValid
    DIRECTION_NULL,     // steering

    false,              // speedValid
    0,                  // leftSpeed
    0,                  // rightSpeed

    false,              // estopValid
    true                // emergencyReleased
};

// ==========================
// Helper functions
// ==========================

static uint16_t read_u16_le(const uint8_t* data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static int32_t read_i32_le(const uint8_t* data)
{
    uint32_t value =
        ((uint32_t)data[0]) |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);

    return (int32_t)value;
}

static int32_t abs_i32(int32_t value)
{
    return value < 0 ? -value : value;
}

static int speed_rpm_to_scale_0_1000(int32_t rpm)
{
    int32_t absRpm = abs_i32(rpm);
    int32_t scaled = absRpm * 1000 / 4500;

    if (scaled > 1000) {
        scaled = 1000;
    }

    return (int)scaled;
}

// ==========================
// Reset vehicle status
// ==========================

void reset_vehicle_status()
{
    vehicleStatus.canConnected = false;

    vehicleStatus.batteryValid = false;
    vehicleStatus.battery = 0.0f;

    vehicleStatus.driveModeValid = false;   // Null
    vehicleStatus.manualMode = false;       // Automatic only after CAN is detected

    vehicleStatus.steeringValid = false;
    vehicleStatus.steering = DIRECTION_NULL;

    vehicleStatus.speedValid = false;
    vehicleStatus.leftSpeed = 0;
    vehicleStatus.rightSpeed = 0;

    vehicleStatus.estopValid = false;
    vehicleStatus.emergencyReleased = true;
}

// ==========================
// Update vehicle mode from state machine
// manualMode = true  -> Manual / HEGE takes control
// manualMode = false -> Automatic / CAN detected but not controlled by HEGE
// ==========================

void update_vehicle_mode(bool manualMode)
{
    vehicleStatus.manualMode = manualMode;
    vehicleStatus.driveModeValid = true;
    vehicleStatus.canConnected = manualMode;
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

    if (abs_i32(leftLogical) <= ZERO_THRESHOLD) leftLogical = 0;
    if (abs_i32(rightLogical) <= ZERO_THRESHOLD) rightLogical = 0;

    if (leftLogical == 0 && rightLogical == 0) {
        vehicleStatus.steering = DIRECTION_NULL;
        vehicleStatus.steeringValid = false;
        return;
    }

    int32_t difference = rightLogical - leftLogical;

    if (abs_i32(difference) <= TURN_THRESHOLD) {
        int32_t average = (leftLogical + rightLogical) / 2;

        if (average > 0) {
            vehicleStatus.steering = DIRECTION_FORWARD;
            vehicleStatus.steeringValid = true;
        }
        else if (average < 0) {
            vehicleStatus.steering = DIRECTION_BACKWARD;
            vehicleStatus.steeringValid = true;
        }
        else {
            vehicleStatus.steering = DIRECTION_NULL;
            vehicleStatus.steeringValid = false;
        }

        return;
    }

    if (difference > TURN_THRESHOLD) {
        vehicleStatus.steering = DIRECTION_LEFT;
        vehicleStatus.steeringValid = true;
        return;
    }

    if (difference < -TURN_THRESHOLD) {
        vehicleStatus.steering = DIRECTION_RIGHT;
        vehicleStatus.steeringValid = true;
        return;
    }

    vehicleStatus.steering = DIRECTION_NULL;
    vehicleStatus.steeringValid = false;
}

// ==========================
// Parse received CAN message
// ==========================

void parse_can_message(uint32_t id, const uint8_t* data, uint8_t dlc)
{
    if (data == nullptr || dlc < 8) {
        return;
    }

    // Any valid CAN message means CAN activity is detected.
    // Before HEGE authentication, drive mode should be Automatic.
    if (!vehicleStatus.canConnected) {
        vehicleStatus.driveModeValid = true;
        vehicleStatus.manualMode = false;
    }

    // ==========================
    // 0x215: Battery / SOC
    // ==========================

    if (id == 0x215) {
        lastBatteryMs = millis();
    
        uint16_t socRaw = read_u16_le(&data[6]);

        vehicleStatus.battery = socRaw * 0.1f;
        vehicleStatus.batteryValid = true;

        return;
    }

    // ==========================
    // 0x315: Left / right speed
    // ==========================

    if (id == 0x315) {
        lastSpeedMs = millis();
        int32_t leftRaw = read_i32_le(&data[0]);
        int32_t rightRaw = read_i32_le(&data[4]);

        vehicleStatus.leftSpeed = speed_rpm_to_scale_0_1000(leftRaw);
        vehicleStatus.rightSpeed = speed_rpm_to_scale_0_1000(rightRaw);

        vehicleStatus.speedValid = true;

        update_steering(leftRaw, rightRaw);

        return;
    }

    // ==========================
    // 0x515: Emergency stop
    // Temporary mapping. Verify later with CAN analyser.
    // ==========================

    if (id == 0x515) {
        lastEstopMs = millis();
        uint8_t estopRaw = data[4] & 0x03;

        vehicleStatus.emergencyReleased = (estopRaw == 0);
        vehicleStatus.estopValid = true;

        return;
    }
}

void update_vehicle_status_timeout()
{
    unsigned long now = millis();

    if (vehicleStatus.batteryValid &&
        now - lastBatteryMs > BATTERY_TIMEOUT_MS) {
        vehicleStatus.batteryValid = false;
        vehicleStatus.battery = 0.0f;
    }

    if (vehicleStatus.speedValid &&
        now - lastSpeedMs > SPEED_TIMEOUT_MS) {
        vehicleStatus.speedValid = false;
        vehicleStatus.leftSpeed = 0;
        vehicleStatus.rightSpeed = 0;

        vehicleStatus.steeringValid = false;
        vehicleStatus.steering = DIRECTION_NULL;
    }

    if (vehicleStatus.estopValid &&
        now - lastEstopMs > ESTOP_TIMEOUT_MS) {
        vehicleStatus.estopValid = false;
        vehicleStatus.emergencyReleased = true;
    }
}