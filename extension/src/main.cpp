#include <Arduino.h>

#include "BoardConfig.h"
#include "InputReader.h"
#include "UartSender.h"
#include "Printer.h"

static InputReader inputReader;
static UartSender uartSender;

static uint32_t nextInputScanUs = 0;
static uint32_t nextUartSendUs = 0;

// 用于限制打印频率
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

    // 每 1 ms 扫描输入
    if (
        static_cast<int32_t>(
            nowUs - nextInputScanUs
        ) >= 0
    ) {
        nextInputScanUs +=
            BoardConfig::INPUT_SCAN_PERIOD_US;

        inputReader.update();
    }

    // 每 10 ms 获取并发送输入状态
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
         * 速度加减事件必须立即打印，
         * 因为下一次 consumeState() 后会被清除。
         */
        if (state.speedUp || state.speedDown) {
            print_physical_input(state);
            printDivider = 0;
        }
        else {
            /*
             * UART 每 10 ms 运行一次。
             * 每累计 10 次打印一次，即每 100 ms 打印一次。
             */
            ++printDivider;

            if (printDivider >= 10) {
                printDivider = 0;

                print_physical_input(state);
            }
        }
    }
}