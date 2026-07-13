#include "PhysicalInput.h"

#include <Arduino.h>

// ================================================================
// UART configuration
// ================================================================

static constexpr int UART_RX_GPIO = 43;
static constexpr int UART_TX_GPIO = 44;

static constexpr uint32_t UART_BAUDRATE = 115200;

// Use UART2
static HardwareSerial inputSerial(2);

// Latest valid input received from external ESP32
static PhysicalInput latestInput;

// ================================================================
// Initialize UART input
// ================================================================

void physical_input_init()
{
    inputSerial.begin(
        UART_BAUDRATE,
        SERIAL_8N1,
        UART_RX_GPIO,
        UART_TX_GPIO
    );

    latestInput = PhysicalInput();
}

// ================================================================
// Read latest UART input packet
// Packet format:
// Byte 0:
// bit0 = forward
// bit1 = backward
// bit2 = left
// bit3 = right
// bit4 = speedUp
// bit5 = speedDown
// bit6 = estop
//
// Byte 1:
// 0x00 = AUTO
// 0x01 = MANUAL
// ================================================================

PhysicalInput read_physical_input()
{
    while (inputSerial.available() >= 2) {

        uint8_t buttons =
            inputSerial.read();

        uint8_t mode =
            inputSerial.read();

        latestInput.forward =
            (buttons & (1 << 0)) != 0;

        latestInput.backward =
            (buttons & (1 << 1)) != 0;

        latestInput.left =
            (buttons & (1 << 2)) != 0;

        latestInput.right =
            (buttons & (1 << 3)) != 0;

        latestInput.speedUp =
            (buttons & (1 << 4)) != 0;

        latestInput.speedDown =
            (buttons & (1 << 5)) != 0;

        latestInput.estop =
            (buttons & (1 << 6)) != 0;

        latestInput.selectedMode =
            (mode == 0x01)
                ? PanelMode::MANUAL
                : PanelMode::AUTO;
    }

    return latestInput;
}