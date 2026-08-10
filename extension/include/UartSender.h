#pragma once

#include <Arduino.h>

#include "InputState.h"

// 实际通过 UART 发送的两个字节
struct UartPacket
{
    // Byte 0：方向、速度事件和急停状态
    uint8_t buttons = 0;

    // Byte 1：0x00 非手动模式，0x01 手动模式
    uint8_t mode = 0;
};

class UartSender
{
public:
    // 初始化与主板之间的 UART
    void begin();

    /*
     * 发送当前输入状态。
     *
     * 返回值是实际发送的两个字节，
     * 供 DebugPrinter 打印。
     */
    UartPacket send(const InputState &state);

private:
    // 使用 ESP32-S3 的 UART2
    HardwareSerial serial_{2};
};