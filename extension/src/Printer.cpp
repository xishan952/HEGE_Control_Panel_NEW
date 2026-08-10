#include "Printer.h"

#include <Arduino.h>

// ================================================================
//serial print initialisation
// ================================================================

void print_init()
{
    Serial.begin(115200);

    delay(300);

}

// ================================================================
// print current input state
// ================================================================

void print_physical_input(const InputState &state)
{
    Serial.print("Manual Mode: ");
    Serial.print(state.manualMode);

    Serial.print(" | Forward: ");
    Serial.print(state.forward);

    Serial.print(" | Backward: ");
    Serial.print(state.backward);

    Serial.print(" | Left: ");
    Serial.print(state.left);

    Serial.print(" | Right: ");
    Serial.print(state.right);

    Serial.print(" | Speed Up: ");
    Serial.print(state.speedUp);

    Serial.print(" | Speed Down: ");
    Serial.print(state.speedDown);

    Serial.print(" | E-Stop: ");
    Serial.println(state.estop);
}