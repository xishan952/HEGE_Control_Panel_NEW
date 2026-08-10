#include "InputReader.h"

#include "BoardConfig.h"

// ================================================================
// 输入编号
// 必须与UART Byte 0的bit位置一致
// ================================================================

static constexpr uint8_t FORWARD_INDEX = 0;
static constexpr uint8_t BACKWARD_INDEX = 1;
static constexpr uint8_t LEFT_INDEX = 2;
static constexpr uint8_t RIGHT_INDEX = 3;
static constexpr uint8_t SPEED_UP_INDEX = 4;
static constexpr uint8_t SPEED_DOWN_INDEX = 5;

// ================================================================
// 六个控制输入GPIO
// ================================================================

static constexpr int CONTROL_PINS[6] = {
    BoardConfig::FORWARD_PIN,
    BoardConfig::BACKWARD_PIN,
    BoardConfig::LEFT_PIN,
    BoardConfig::RIGHT_PIN,
    BoardConfig::SPEED_UP_PIN,
    BoardConfig::SPEED_DOWN_PIN
};

// ================================================================
// 读取低电平有效输入
// ================================================================

static bool readActiveLow(int pin)
{
    return digitalRead(pin) == LOW;
}

// ================================================================
// 初始化GPIO
// ================================================================

void InputReader::begin()
{
    pinMode(BoardConfig::FORWARD_PIN, INPUT_PULLUP);
    pinMode(BoardConfig::BACKWARD_PIN, INPUT_PULLUP);
    pinMode(BoardConfig::LEFT_PIN, INPUT_PULLUP);
    pinMode(BoardConfig::RIGHT_PIN, INPUT_PULLUP);

    pinMode(BoardConfig::SPEED_UP_PIN, INPUT_PULLUP);
    pinMode(BoardConfig::SPEED_DOWN_PIN, INPUT_PULLUP);

    pinMode(BoardConfig::ESTOP_STATUS_PIN, INPUT_PULLUP);

    // GPIO12：手动模式开关
    pinMode(BoardConfig::MANUAL_MODE_PIN, INPUT_PULLUP);

    stableControlMask_ = 0;

    manualMode_ = false;
    manualModeCounter_ = 0;

    estopActive_ = false;

    speedUpEvent_ = false;
    speedDownEvent_ = false;

    for (uint8_t i = 0; i < 6; ++i) {
        controlCounters_[i] = 0;
    }
}

// ================================================================
// 每1 ms更新输入状态
// ================================================================

void InputReader::update()
{
    // ------------------------------------------------------------
    // 1. 读取六个按钮并消抖
    // ------------------------------------------------------------

    for (uint8_t i = 0; i < 6; ++i) {

        const uint8_t bit =
            static_cast<uint8_t>(1U << i);

        // 修改前的稳定状态
        const bool wasStableActive =
            (stableControlMask_ & bit) != 0;

        // 读取真实GPIO电平
        const bool rawActive =
            readActiveLow(CONTROL_PINS[i]);

        // --------------------------------------------------------
        // 消抖计数
        // --------------------------------------------------------

        if (rawActive) {
            if (
                controlCounters_[i] <
                BoardConfig::DEBOUNCE_COUNT
            ) {
                ++controlCounters_[i];
            }
        }
        else {
            if (controlCounters_[i] > 0) {
                --controlCounters_[i];
            }
        }

        // 连续稳定按下后，把对应位设为1
        if (
            controlCounters_[i] >=
            BoardConfig::DEBOUNCE_COUNT
        ) {
            stableControlMask_ |= bit;
        }

        // 完全稳定松开后，把对应位清零
        if (controlCounters_[i] == 0) {
            stableControlMask_ &=
                static_cast<uint8_t>(~bit);
        }

        // 修改后的稳定状态
        const bool isStableActive =
            (stableControlMask_ & bit) != 0;

        // --------------------------------------------------------
        // 检测稳定状态的 false → true
        //
        // 电气上对应 HIGH → LOW：
        // 开关从未按下变成按下
        // --------------------------------------------------------

        const bool pressedEdge =
            !wasStableActive && isStableActive;

        if (pressedEdge) {
            if (i == SPEED_UP_INDEX) {
                speedUpEvent_ = true;
            }

            if (i == SPEED_DOWN_INDEX) {
                speedDownEvent_ = true;
            }
        }
    }

    // ------------------------------------------------------------
    // 2. 读取GPIO12手动模式开关
    // ------------------------------------------------------------

    const bool manualSwitchActive =
        readActiveLow(BoardConfig::MANUAL_MODE_PIN);

    if (!manualSwitchActive) {
        /*
         * 开关断开：
         * 立即退出手动模式。
         */
        manualMode_ = false;
        manualModeCounter_ = 0;

        /*
         * 防止在关闭模式后，
         * 之前尚未发送的速度事件继续生效。
         */
        speedUpEvent_ = false;
        speedDownEvent_ = false;
    }
    else if (!manualMode_) {
        /*
         * 开关闭合：
         * 稳定约20 ms后进入手动模式。
         */
        if (
            manualModeCounter_ <
            BoardConfig::DEBOUNCE_COUNT
        ) {
            ++manualModeCounter_;
        }

        if (
            manualModeCounter_ >=
            BoardConfig::DEBOUNCE_COUNT
        ) {
            manualMode_ = true;
        }
    }

    // ------------------------------------------------------------
    // 3. 急停状态始终读取
    // ------------------------------------------------------------

    estopActive_ =
        readActiveLow(BoardConfig::ESTOP_STATUS_PIN);
}

// ================================================================
// 获取状态并消费一次性事件
// ================================================================

InputState InputReader::consumeState()
{
    InputState state;

    state.manualMode = manualMode_;
    state.estop = estopActive_;

    if (manualMode_) {
        /*
         * 方向按钮是持续状态。
         *
         * 只要按钮一直按着，对应值就一直是true。
         */
        state.forward =
            (stableControlMask_ &
             (1U << FORWARD_INDEX)) != 0;

        state.backward =
            (stableControlMask_ &
             (1U << BACKWARD_INDEX)) != 0;

        state.left =
            (stableControlMask_ &
             (1U << LEFT_INDEX)) != 0;

        state.right =
            (stableControlMask_ &
             (1U << RIGHT_INDEX)) != 0;

        /*
         * 速度按钮不是持续状态，
         * 而是一次性按下事件。
         */
        state.speedUp = speedUpEvent_;
        state.speedDown = speedDownEvent_;
    }

    /*
     * 事件已经交给UART发送模块，
     * 立即清除。
     *
     * 按钮保持按下不会再次触发，
     * 必须先松开再重新按下。
     */
    speedUpEvent_ = false;
    speedDownEvent_ = false;

    return state;
}