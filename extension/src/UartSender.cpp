#include "UartSender.h"

#include "BoardConfig.h"

// ================================================================
// 初始化 UART
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
// 生成并发送两字节输入报文
// ================================================================

UartPacket UartSender::send(const InputState &state)
{
    UartPacket packet;

    // ------------------------------------------------------------
    // Byte 0：按钮和急停状态
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
    // Byte 1：模式
    // ------------------------------------------------------------

    packet.mode =
        state.manualMode ? 0x01 : 0x00;

    // ------------------------------------------------------------
    // 连续发送两个原始字节
    // ------------------------------------------------------------

    const uint8_t rawPacket[2] = {
        packet.buttons,
        packet.mode
    };

    serial_.write(
        rawPacket,
        sizeof(rawPacket)
    );

    // 返回实际发送的数据，供调试模块打印
    return packet;
}