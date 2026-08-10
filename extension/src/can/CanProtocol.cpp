#include "CanProtocol.h"
#include "../config/AppConfig.h"

// ================================================================
// Helper functions
// ================================================================

static uint16_t readU16LittleEndian(const uint8_t* data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static int32_t readI32LittleEndian(const uint8_t* data) {
    uint32_t value =
        ((uint32_t)data[0]) |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);

    return (int32_t)value;
}

static void writeU16LittleEndian(uint8_t* data, uint16_t value) {
    data[0] = (uint8_t)(value & 0xFF);
    data[1] = (uint8_t)((value >> 8) & 0xFF);
}

static uint16_t limitSpeedMagnitude(uint16_t speed) {
    if (speed > 1000) {
        return 1000;
    }

    return speed;
}

static uint8_t limitGear(uint8_t gear) {
    if (gear > 4) {
        return 4;
    }

    return gear;
}

// ================================================================
// RX parsing: 0x215
// ================================================================

bool parseStatus215(const CanFrame& frame, MainSystemFeedback& feedback) {
    if (frame.id != CAN_ID_STATUS_215) {
        return false;
    }

    if (frame.dlc < 8) {
        return false;
    }

    feedback.activationStatus = frame.data[0];
    feedback.randomNumber = frame.data[1];
    feedback.shiftValue = frame.data[2];

    feedback.outputFeedback = frame.data[4];
    feedback.currentGear = frame.data[5];

    feedback.socRaw = readU16LittleEndian(&frame.data[6]);
    feedback.socPercent = feedback.socRaw * 0.1f;

    return true;
}

// ================================================================
// RX parsing: 0x315
// ================================================================

bool parseSpeed315(const CanFrame& frame, MainSystemFeedback& feedback) {
    if (frame.id != CAN_ID_SPEED_315) {
        return false;
    }

    if (frame.dlc < 8) {
        return false;
    }

    feedback.speedLeft = readI32LittleEndian(&frame.data[0]);
    feedback.speedRight = readI32LittleEndian(&frame.data[4]);

    return true;
}

// ================================================================
// TX builder: 0x195 external control request
// ================================================================

CanFrame buildExternalControlRequest195() {
    CanFrame frame;

    frame.id = CAN_ID_AUTH_195;
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
// TX builder: 0x195 authentication reply
// ================================================================

CanFrame buildAuthReply195(uint8_t randomNumber, uint8_t shiftValue) {
    CanFrame frame;

    frame.id = CAN_ID_AUTH_195;
    frame.dlc = 8;

    if (shiftValue > 7) {
        shiftValue = 7;
    }

    uint8_t authResult = randomNumber >> shiftValue;

    frame.data[0] = 0x02;
    frame.data[1] = 0x00;
    frame.data[2] = 0x00;
    frame.data[3] = authResult;
    frame.data[4] = 0x00;
    frame.data[5] = 0x00;
    frame.data[6] = 0x00;
    frame.data[7] = 0x00;

    return frame;
}

// ================================================================
// TX builder: 0x195 deactivate external control
// ================================================================

CanFrame buildDeactivate195() {
    CanFrame frame;

    frame.id = CAN_ID_AUTH_195;
    frame.dlc = 8;

    for (int i = 0; i < 8; i++) {
        frame.data[i] = 0x00;
    }

    return frame;
}

// ================================================================
// TX builder: 0x295 neutral / stop
// ================================================================

CanFrame buildNeutral295() {
    CanFrame frame;

    frame.id = CAN_ID_CONTROL_295;
    frame.dlc = 8;

    frame.data[0] = 0x03;
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
// TX builder: 0x295 manual crawler control
// ================================================================

CanFrame buildManualControl295(const ManualControlCommand& command) {
    CanFrame frame;

    frame.id = CAN_ID_CONTROL_295;
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

    uint16_t speed = limitSpeedMagnitude(command.speedMagnitude);
    uint8_t gear = limitGear(command.gear);

    if (command.movement == MovementCommand::NEUTRAL || speed == 0) {
        dirLeft = 0;
        dirRight = 0;
        speed = 0;
        gear = 0;
    }

    frame.data[0] = (uint8_t)(0x03 | (dirLeft << 4) | (dirRight << 6));

    writeU16LittleEndian(&frame.data[1], speed);
    writeU16LittleEndian(&frame.data[3], speed);

    frame.data[5] = gear;
    frame.data[6] = 0x00;
    frame.data[7] = 0x00;

    return frame;
}