#include "InputReader.h"

#include "BoardConfig.h"

// ================================================================
// input index for six control inputs
// ================================================================

static constexpr uint8_t FORWARD_INDEX = 0;
static constexpr uint8_t BACKWARD_INDEX = 1;
static constexpr uint8_t LEFT_INDEX = 2;
static constexpr uint8_t RIGHT_INDEX = 3;
static constexpr uint8_t SPEED_UP_INDEX = 4;
static constexpr uint8_t SPEED_DOWN_INDEX = 5;

// ================================================================
// six control input GPIOs
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
// read GPIO pin with active low logic
// ================================================================

static bool readActiveLow(int pin)
{
    return digitalRead(pin) == LOW;
}

// ================================================================
// GPIO initialization
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
// updsate physical inputs and internal state
// ================================================================

void InputReader::update()
{
    // ------------------------------------------------------------
    // 1. read 6 inputs
    // ------------------------------------------------------------

    for (uint8_t i = 0; i < 6; ++i) {

        const uint8_t bit =
            static_cast<uint8_t>(1U << i);

        // stable state before update
        const bool wasStableActive =
            (stableControlMask_ & bit) != 0;

        // read raw GPIO level
        const bool rawActive =
            readActiveLow(CONTROL_PINS[i]);

        // --------------------------------------------------------
        // debounce counter
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

        // keeping stable state after debouncing
        if (
            controlCounters_[i] >=
            BoardConfig::DEBOUNCE_COUNT
        ) {
            stableControlMask_ |= bit;
        }

        // clearing stable state after debouncing
        if (controlCounters_[i] == 0) {
            stableControlMask_ &=
                static_cast<uint8_t>(~bit);
        }

        // stable state after update
        const bool isStableActive =
            (stableControlMask_ & bit) != 0;

        // --------------------------------------------------------
        // test for rising edge (HIGH → LOW)
        // switch is active low
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
    // 2. read manual mode switch
    // ------------------------------------------------------------

    const bool manualSwitchActive =
        readActiveLow(BoardConfig::MANUAL_MODE_PIN);

    if (!manualSwitchActive) {
        /*
         *manual mode turned off
         */
        manualMode_ = false;
        manualModeCounter_ = 0;

        speedUpEvent_ = false;
        speedDownEvent_ = false;
    }
    else if (!manualMode_) {
        /*
         *manual mode turned on after 20ms
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
    // 3. read e-stop
    // ------------------------------------------------------------

    estopActive_ =
        readActiveLow(BoardConfig::ESTOP_STATUS_PIN);
}

InputState InputReader::consumeState()
{
    InputState state;

    state.manualMode = manualMode_;
    state.estop = estopActive_;

    if (manualMode_) {
        /*
         * keep stable state after debouncing
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
         * speedUp/speedDown events are not stable states after debouncing,
         */
        state.speedUp = speedUpEvent_;
        state.speedDown = speedDownEvent_;
    }

    /*
     * send the event state to the UART sending program, and clear it immediately.
     */
    speedUpEvent_ = false;
    speedDownEvent_ = false;

    return state;
}