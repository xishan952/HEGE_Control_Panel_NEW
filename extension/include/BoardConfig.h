#pragma once

#include <Arduino.h>

namespace BoardConfig
{
    // ============================================================
    //  GPIO for physical inputs
    // ============================================================

    constexpr int FORWARD_PIN = 4;
    constexpr int BACKWARD_PIN = 5;
    constexpr int LEFT_PIN = 6;
    constexpr int RIGHT_PIN = 7;

    constexpr int SPEED_UP_PIN = 8;
    constexpr int SPEED_DOWN_PIN = 9;

    constexpr int ESTOP_STATUS_PIN = 10;

    // GPIO11 is reserved for future use, currently not used
    constexpr int UNUSED_GPIO_11 = 11;

    // manual mode GPIO12
    constexpr int MANUAL_MODE_PIN = 12;

    // ============================================================
    // don't use
    // ============================================================

    constexpr int ENCODER_DT_PIN = 13;
    constexpr int BUZZER_PIN = 15;
    constexpr int FAULT_LED_PIN = 16;
    //constexpr int ENCODER_SW_PIN = 17;
    //constexpr int RESERVED_PIN = 18;

    // ============================================================
    // UART communication with main board
    // ============================================================

    constexpr int UART_RX_PIN = 18;
    constexpr int UART_TX_PIN = 17;

    constexpr uint32_t UART_BAUDRATE = 115200;

    // ============================================================
    // time cycles
    // ============================================================

    // scan physical inputs every 1 ms
    constexpr uint32_t INPUT_SCAN_PERIOD_US = 1000;

    // send data to main board every 10 ms
    constexpr uint32_t UART_SEND_PERIOD_US = 10000;

    // debounce time fornormal buttons and manual switches is about 20 ms
    constexpr uint8_t DEBOUNCE_COUNT = 20;
}