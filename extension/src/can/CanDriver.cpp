#include "CanDriver.h"
#include "../config/AppConfig.h"

#include "driver/twai.h"
#include "driver/gpio.h"

// ==========================
// Internal driver state
// ==========================

static CanDriverState driverState = CanDriverState::NOT_CONFIGURED;

// ==========================
// Check GPIO configuration
// ==========================

static bool gpio_config_available()
{
    return (CAN_TX_GPIO >= 0 && CAN_RX_GPIO >= 0);
}

// ==========================
// CAN driver initialization
// ==========================

bool can_driver_init()
{
    Serial.println("[CAN] init entered");
    Serial.flush();

    if (!gpio_config_available()) {
        Serial.println("[CAN] GPIO not configured. CAN driver disabled.");
        Serial.flush();
        driverState = CanDriverState::NOT_CONFIGURED;
        return false;
    }

    Serial.print("[CAN] TX GPIO = ");
    Serial.println(CAN_TX_GPIO);
    Serial.print("[CAN] RX GPIO = ");
    Serial.println(CAN_RX_GPIO);
    Serial.flush();

    twai_general_config_t generalConfig =
        TWAI_GENERAL_CONFIG_DEFAULT(
            (gpio_num_t)CAN_TX_GPIO,
            (gpio_num_t)CAN_RX_GPIO,
            TWAI_MODE_NORMAL
        );

    twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    Serial.println("[CAN] before twai_driver_install");
    Serial.flush();

    esp_err_t installResult =
        twai_driver_install(&generalConfig, &timingConfig, &filterConfig);

    Serial.println("[CAN] after twai_driver_install");
    Serial.flush();

    if (installResult != ESP_OK) {
        Serial.print("[CAN] TWAI driver install failed. Error: ");
        Serial.println((int)installResult);
        Serial.flush();
        driverState = CanDriverState::ERROR;
        return false;
    }

    Serial.println("[CAN] before twai_start");
    Serial.flush();

    esp_err_t startResult = twai_start();

    Serial.println("[CAN] after twai_start");
    Serial.flush();

    if (startResult != ESP_OK) {
        Serial.print("[CAN] TWAI start failed. Error: ");
        Serial.println((int)startResult);
        Serial.flush();
        driverState = CanDriverState::ERROR;
        return false;
    }

    Serial.println("[CAN] TWAI driver started.");
    Serial.flush();
    driverState = CanDriverState::READY;
    return true;
}

// ==========================
// CAN driver status
// ==========================

bool can_driver_is_ready()
{
    return driverState == CanDriverState::READY;
}

CanDriverState can_driver_get_state()
{
    return driverState;
}

// ==========================
// CAN send
// ==========================

bool can_driver_send(const CanFrame& frame)
{
    if (!can_driver_is_ready()) {
        return false;
    }

    twai_message_t message = {};
    message.identifier = frame.id;
    message.extd = 0;
    message.rtr = 0;
    message.data_length_code = frame.dlc;

    for (uint8_t i = 0; i < frame.dlc && i < 8; i++) {
        message.data[i] = frame.data[i];
    }

    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(5));

    return result == ESP_OK;
}

// ==========================
// CAN receive
// ==========================

bool can_driver_receive(CanFrame& frame)
{
    if (!can_driver_is_ready()) {
        return false;
    }

    twai_message_t message;

    esp_err_t result = twai_receive(&message, 0);

    if (result != ESP_OK) {
        return false;
    }

    if (message.extd || message.rtr) {
        return false;
    }

    frame.id = message.identifier;
    frame.dlc = message.data_length_code;

    for (uint8_t i = 0; i < frame.dlc && i < 8; i++) {
        frame.data[i] = message.data[i];
    }

    return true;
}