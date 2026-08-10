#include "UartSender.h"

#include "BoardConfig.h"

// ================================================================
// UART initialization
// ================================================================

void UartSender::begin()
{
    serial_.begin(
        BoardConfig::UART_BAUDRATE,
        SERIAL_8N1,
        BoardConfig::UART_RX_PIN,
        BoardConfig::UART_TX_PIN
    );
}

// ================================================================
// create and send a packet to main board
// ================================================================

UartPacket UartSender::send(const InputState &state)
{
    UartPacket packet;

    // ------------------------------------------------------------
    // Byte 0：speed events, direction and e-stop
    // ------------------------------------------------------------

    if (state.forward) {
        packet.buttons |= (1U << 0);
    }

    if (state.backward) {
        packet.buttons |= (1U << 1);
    }

    if (state.left) {
        packet.buttons |= (1U << 2);
    }

    if (state.right) {
        packet.buttons |= (1U << 3);
    }

    if (state.speedUp) {
        packet.buttons |= (1U << 4);
    }

    if (state.speedDown) {
        packet.buttons |= (1U << 5);
    }

    if (state.estop) {
        packet.buttons |= (1U << 6);
    }

    // ------------------------------------------------------------
    // Byte 1：mode
    // ------------------------------------------------------------

    packet.mode =
        state.manualMode ? 0x01 : 0x00;

    // ------------------------------------------------------------
    // keep sending the packet to main board
    // ------------------------------------------------------------

    const uint8_t rawPacket[2] = {
        packet.buttons,
        packet.mode
    };

     Serial.printf("SEND buttons=0x%02X mode=0x%02X\n", packet.buttons, packet.mode);

    serial_.write(
        rawPacket,
        sizeof(rawPacket)
    );

    // send real inputs back
    return packet;
}