#include <Arduino.h>

#include "BoardConfig.h"
#include "InputReader.h"
#include "UartSender.h"
#include "Printer.h"

static InputReader inputReader;
static UartSender uartSender;

static uint32_t nextInputScanUs = 0;
static uint32_t nextUartSendUs = 0;
static uint8_t printDivider = 0;

void setup()
{
    print_init();

    inputReader.begin();
    uartSender.begin();

    const uint32_t nowUs = micros();

    nextInputScanUs =
        nowUs + BoardConfig::INPUT_SCAN_PERIOD_US;

    nextUartSendUs =
        nowUs + BoardConfig::UART_SEND_PERIOD_US;
}

void loop()
{
    const uint32_t nowUs = micros();

    //scan the input in 10ms
    if (
        static_cast<int32_t>(
            nowUs - nextInputScanUs
        ) >= 0
    ) {
        nextInputScanUs +=
            BoardConfig::INPUT_SCAN_PERIOD_US;

        inputReader.update();
    }

    // send the input state to main board in 10ms
    if (
        static_cast<int32_t>(
            nowUs - nextUartSendUs
        ) >= 0
    ) {
        nextUartSendUs +=
            BoardConfig::UART_SEND_PERIOD_US;

        const InputState state =
            inputReader.consumeState();

        uartSender.send(state);

        /*
         * print speedUp/speedDown events immediately,
         * because they will be cleared after the next consumeState().
         */
        if (state.speedUp || state.speedDown) {
            print_physical_input(state);
            printDivider = 0;
        }
        else {
            /*
             * run UART sending program every 10 ms, but only print every 100 ms.
             */
            ++printDivider;

            if (printDivider >= 10) {
                printDivider = 0;

                print_physical_input(state);
            }
        }
    }
}