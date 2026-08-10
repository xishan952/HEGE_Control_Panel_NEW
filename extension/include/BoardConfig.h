#pragma once

#include <Arduino.h>

namespace BoardConfig
{
    // ============================================================
    // 物理输入 GPIO
    // ============================================================

    constexpr int FORWARD_PIN = 4;
    constexpr int BACKWARD_PIN = 5;
    constexpr int LEFT_PIN = 6;
    constexpr int RIGHT_PIN = 7;

    constexpr int SPEED_UP_PIN = 8;
    constexpr int SPEED_DOWN_PIN = 9;

    // 急停辅助状态输入
    constexpr int ESTOP_STATUS_PIN = 10;

    // GPIO11 当前不使用
    constexpr int UNUSED_GPIO_11 = 11;

    // 手动模式开关，固定为 GPIO12
    constexpr int MANUAL_MODE_PIN = 12;

    // ============================================================
    // 暂时保留但当前版本不使用
    // ============================================================

    constexpr int ENCODER_DT_PIN = 13;
    constexpr int BUZZER_PIN = 15;
    constexpr int FAULT_LED_PIN = 16;
    constexpr int ENCODER_SW_PIN = 17;
    constexpr int RESERVED_PIN = 18;

    // ============================================================
    // 与主板通信的 UART
    // ============================================================

    constexpr int UART_RX_PIN = 43;
    constexpr int UART_TX_PIN = 44;

    constexpr uint32_t UART_BAUDRATE = 115200;

    // ============================================================
    // 程序周期
    // ============================================================

    // 每 1 ms 扫描一次物理输入
    constexpr uint32_t INPUT_SCAN_PERIOD_US = 1000;

    // 每 10 ms 向主板发送一次
    constexpr uint32_t UART_SEND_PERIOD_US = 10000;

    // 普通按钮和手动开关消抖时间约 20 ms
    constexpr uint8_t DEBOUNCE_COUNT = 20;
}