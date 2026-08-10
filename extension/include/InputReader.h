#pragma once

#include <Arduino.h>

#include "InputState.h"

class InputReader
{
public:
    // 初始化所有输入GPIO
    void begin();

    // 每1 ms读取和处理一次输入
    void update();

    /*
     * 获取当前输入状态。
     *
     * speedUp和speedDown是一次性事件：
     * 返回后会自动清除。
     */
    InputState consumeState();

private:
    // 六个输入经过消抖后的稳定状态
    uint8_t stableControlMask_ = 0;

    // 六个输入的消抖计数器
    uint8_t controlCounters_[6] = {};

    // 手动模式开关
    bool manualMode_ = false;
    uint8_t manualModeCounter_ = 0;

    // 急停状态
    bool estopActive_ = false;

    /*
     * 速度按键事件。
     *
     * 一旦检测到一次HIGH→LOW，就保持为true，
     * 直到UART发送程序读取并清除。
     */
    bool speedUpEvent_ = false;
    bool speedDownEvent_ = false;
};