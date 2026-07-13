#include "CanProtocol.h"
#include "../config/AppConfig.h"
#include "../ui/VehicleStatus.h"

// ================================================================
// RX timing state
// ================================================================

static unsigned long lastCanMessageMs = 0;
static unsigned long lastBatteryMs = 0;
static unsigned long lastSpeedMs = 0;

static bool canMessageReceived = false;

static const unsigned long CAN_TIMEOUT_MS = 1000;
static const unsigned long BATTERY_TIMEOUT_MS = 1000;
static const unsigned long SPEED_TIMEOUT_MS = 500;

// ================================================================
// Control response state
// ================================================================

bool controlAnswerOk = false;

// ================================================================
// Helper functions
// ================================================================

static uint16_t readU16LittleEndian(const uint8_t* data)
{
    return (uint16_t)data[0] |
           ((uint16_t)data[1] << 8);
}

static int32_t readI32LittleEndian(const uint8_t* data)
{
    uint32_t value =
        ((uint32_t)data[0]) |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);

    return (int32_t)value;
}

static void writeU16LittleEndian(uint8_t* data, uint16_t value)
{
    data[0] = (uint8_t)(value & 0xFF);
    data[1] = (uint8_t)((value >> 8) & 0xFF);
}

static int32_t absI32(int32_t value)
{
    return value < 0 ? -value : value;
}

static int speedRpmToScale0To1000(int32_t rpm)
{
    int32_t absRpm = absI32(rpm);
    int32_t scaled = absRpm * 1000 / 4500;

    if (scaled > 1000) {
        scaled = 1000;
    }

    return (int)scaled;
}

static uint16_t limitSpeedMagnitude(uint16_t speed)
{
    if (speed > 1000) {
        return 1000;
    }

    return speed;
}

static uint8_t limitGear(uint8_t gear)
{
    if (gear > 4) {
        return 4;
    }

    return gear;
}

// ================================================================
// RX parsing
// ================================================================

void parse_can_message(
    uint32_t id,
    const uint8_t* data,
    uint8_t dlc
)
{
    if (data == nullptr || dlc < 8) {
        return;
    }

    // Any valid CAN message means CAN communication is active.
    lastCanMessageMs = millis();
    canMessageReceived = true;

    // ============================================================
    // 0x140: Manual control request response
    // Byte 0 == 0x01 means answer OK
    // ============================================================

    if (id == CAN_ID_CONTROL_140) {

        controlAnswerOk =
            (data[0] == 0x01);

        return;
    }

    // ============================================================
    // 0x215: Battery / SOC and remote control source
    //
    // Byte 0 == 0x03 means remote controller is active.
    // PANEL CONTROL has higher priority and must not be overwritten.
    // ============================================================

    if (id == CAN_ID_STATUS_215) {
        lastBatteryMs = millis();

        if (data[0] == 0x03 &&
            !vehicleStatus.manualMode) {

            vehicleStatus.controlSource =
                CONTROL_SOURCE_REMOTE;
        }

        uint16_t socRaw =
            readU16LittleEndian(&data[6]);

        vehicleStatus.battery =
            socRaw * 0.1f;

        vehicleStatus.batteryValid =
            true;

        return;
    }

    // ============================================================
    // 0x315: Left / right speed
    // ============================================================

    if (id == CAN_ID_SPEED_315) {
        lastSpeedMs = millis();

        int32_t leftRaw =
            readI32LittleEndian(&data[0]);

        int32_t rightRaw =
            readI32LittleEndian(&data[4]);

        vehicleStatus.leftSpeed =
            speedRpmToScale0To1000(leftRaw);

        vehicleStatus.rightSpeed =
            speedRpmToScale0To1000(rightRaw);

        vehicleStatus.speedValid =
            true;

        update_steering(
            leftRaw,
            rightRaw
        );

        return;
    }
}

// ================================================================
// RX timeout handling
// ================================================================

void update_vehicle_status_timeout()
{
    unsigned long now = millis();

    // ------------------------------------------------------------
    // No CAN messages detected
    // ------------------------------------------------------------

    if (!canMessageReceived ||
        now - lastCanMessageMs > CAN_TIMEOUT_MS) {

        vehicleStatus.controlSource =
            CONTROL_SOURCE_ERROR;
    }

    // ------------------------------------------------------------
    // Battery timeout
    // ------------------------------------------------------------

    if (vehicleStatus.batteryValid &&
        now - lastBatteryMs > BATTERY_TIMEOUT_MS) {

        vehicleStatus.batteryValid = false;
        vehicleStatus.battery = 0.0f;
    }

    // ------------------------------------------------------------
    // Speed timeout
    // ------------------------------------------------------------

    if (vehicleStatus.speedValid &&
        now - lastSpeedMs > SPEED_TIMEOUT_MS) {

        vehicleStatus.speedValid = false;
        vehicleStatus.leftSpeed = 0;
        vehicleStatus.rightSpeed = 0;

        vehicleStatus.steeringValid = false;
        vehicleStatus.steering =
            DIRECTION_NULL;
    }
}

// ================================================================
// TX builder: 0x150 manual control request
// ================================================================

CanFrame buildControlRequest150()
{
    CanFrame frame;

    frame.id = CAN_ID_CONTROL_150;
    frame.dlc = 8;

    frame.data[0] = 0x02;
    frame.data[1] = 0x00;
    frame.data[2] = 0x00;
    frame.data[3] = 0x00;
    frame.data[4] = 0x00;
    frame.data[5] = 0x00;
    frame.data[6] = 0x00;
    frame.data[7] = 0x00;

    return frame;
}

// ================================================================
// TX builder: 0x150 emergency stop command
// ================================================================

CanFrame buildEstop150()
{
    CanFrame frame;

    frame.id = CAN_ID_CONTROL_150;
    frame.dlc = 8;

    frame.data[0] = 0x00;
    frame.data[1] = 0x00;
    frame.data[2] = 0x00;
    frame.data[3] = 0x00;
    frame.data[4] = 0x00;
    frame.data[5] = 0x00;
    frame.data[6] = 0x00;
    frame.data[7] = 0xFF;

    return frame;
}

// ================================================================
// TX builder: 0x330 manual speed command
// ================================================================

CanFrame buildSpeedCommand330(
    const ManualControlCommand& command
)
{
    CanFrame frame;

    frame.id = CAN_ID_SPEED_330;
    frame.dlc = 8;

    uint8_t dirLeft = 0;
    uint8_t dirRight = 0;

    switch (command.movement) {

        case MovementCommand::FORWARD:
            dirLeft = 1;
            dirRight = 1;
            break;

        case MovementCommand::BACKWARD:
            dirLeft = 2;
            dirRight = 2;
            break;

        case MovementCommand::LEFT_TURN:
            dirLeft = 2;
            dirRight = 1;
            break;

        case MovementCommand::RIGHT_TURN:
            dirLeft = 1;
            dirRight = 2;
            break;

        case MovementCommand::NEUTRAL:
        default:
            dirLeft = 0;
            dirRight = 0;
            break;
    }

    uint16_t speed =
        limitSpeedMagnitude(
            command.speedMagnitude
        );

    uint8_t gear =
        limitGear(
            command.gear
        );

    if (command.movement ==
            MovementCommand::NEUTRAL ||
        speed == 0) {

        dirLeft = 0;
        dirRight = 0;
        speed = 0;
        gear = 0;
    }

    frame.data[0] =
        (uint8_t)(
            0x03 |
            (dirLeft << 4) |
            (dirRight << 6)
        );

    writeU16LittleEndian(
        &frame.data[1],
        speed
    );

    writeU16LittleEndian(
        &frame.data[3],
        speed
    );

    frame.data[5] = gear;
    frame.data[6] = 0x00;
    frame.data[7] = 0x00;

    return frame;
}