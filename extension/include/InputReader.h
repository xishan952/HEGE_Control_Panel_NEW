#pragma once

#include <Arduino.h>

#include "InputState.h"

class InputReader
{
public:
    // initialisation
    void begin();

    // read physical inputs and update internal state
    void update();

    /*
     * reach current inputs
     *
     * automatically clear speedUp/speedDown events after reading
     */
    InputState consumeState();

private:
    // six stable states after debouncing
    uint8_t stableControlMask_ = 0;

    // debouncing counters for six inputs
    uint8_t controlCounters_[6] = {};

    // manual mode state and counter for debouncing
    bool manualMode_ = false;
    uint8_t manualModeCounter_ = 0;

    // emergency stop state
    bool estopActive_ = false;

    /*
     *speedUp/speedDown events
     *
     *once set, these events will remain true until the UART sending program reads and clears them.
     * until the UART sending program reads and clears them.
     */
    bool speedUpEvent_ = false;
    bool speedDownEvent_ = false;
};