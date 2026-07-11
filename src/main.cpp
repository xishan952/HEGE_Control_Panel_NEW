#include <Arduino.h>

#include "can/CanDriver.h"
#include "can/CanFrame.h"
#include "ui/VehicleStatus.h"

#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "ui/HEGE_UI.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static unsigned long lastPrintMs = 0;

static void printByteHex(uint8_t value)
{
    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);
}

static void printCanFrame(const CanFrame& frame)
{
    Serial.print("[RX] ID=0x");
    Serial.print(frame.id, HEX);
    Serial.print(" DLC=");
    Serial.print(frame.dlc);
    Serial.print(" DATA=");

    for (uint8_t i = 0; i < frame.dlc && i < 8; i++) {
        printByteHex(frame.data[i]);
        Serial.print(" ");
    }

    Serial.println();
}

static void printVehicleStatus()
{
    Serial.println("---------- VehicleStatus ----------");

    Serial.print("CAN connected: ");
    Serial.println(vehicleStatus.canConnected ? "true" : "false");

    Serial.print("Battery valid: ");
    Serial.println(vehicleStatus.batteryValid ? "true" : "false");

    Serial.print("Battery: ");
    Serial.println(vehicleStatus.battery);

    Serial.print("Speed valid: ");
    Serial.println(vehicleStatus.speedValid ? "true" : "false");

    Serial.print("Left speed: ");
    Serial.println(vehicleStatus.leftSpeed);

    Serial.print("Right speed: ");
    Serial.println(vehicleStatus.rightSpeed);

    Serial.print("Steering valid: ");
    Serial.println(vehicleStatus.steeringValid ? "true" : "false");

    Serial.print("Steering enum: ");
    Serial.println((int)vehicleStatus.steering);

    Serial.print("Manual mode: ");
    Serial.println(vehicleStatus.manualMode ? "true" : "false");

    Serial.println("-----------------------------------");
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("===== HEGE MAIN START =====");

    Serial.println("Initializing board");

    Board *board = new Board();
    board->init();

#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);

#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(
            lcd->getFrameWidth() * 10);
    }
#endif
#endif

    Serial.println("Before board begin");
    bool boardReady = board->begin();
    Serial.println("After board begin");

    if (!boardReady) {
        Serial.println("[INIT] Board begin failed.");
    }

    Serial.println("Before reset_vehicle_status");
    reset_vehicle_status();
    Serial.println("After reset_vehicle_status");

    Serial.println("Before CAN init");
    bool canReady = can_driver_init();
    Serial.println("After CAN init");

    if (canReady) {
        Serial.println("[INIT] CAN ready.");
    } else {
        Serial.println("[INIT] CAN not ready.");
    }

    // 不要开机默认 Automatic
    // update_vehicle_mode(false);

    Serial.println("Before LVGL init");
    lvgl_port_init(board->getLCD(), board->getTouch());
    Serial.println("After LVGL init");

    Serial.println("Before create HEGE UI");

    lvgl_port_lock(-1);

    create_hege_ui();
    update_hege_ui(nullptr);
    lv_timer_create(update_hege_ui, 200, nullptr);

    lvgl_port_unlock();

    Serial.println("HEGE UI started");
}

void loop()
{
    CanFrame rxFrame;

    while (can_driver_receive(rxFrame)) {
        printCanFrame(rxFrame);

        parse_can_message(
            rxFrame.id,
            rxFrame.data,
            rxFrame.dlc
        );
    }
    update_vehicle_status_timeout();

    if (millis() - lastPrintMs >= 1000) {
        lastPrintMs = millis();
        Serial.println("loop alive");
        printVehicleStatus();
    }

    delay(5);
}