#include <Arduino.h>
#include <lvgl.h>

#include "HEGE_UI.h"
#include "VehicleStatus.h"

// ==========================
// HEGE UI label objects
// ==========================
static lv_obj_t *label_can;
static lv_obj_t *label_battery;
static lv_obj_t *label_drive;
static lv_obj_t *label_steering;
static lv_obj_t *label_speed;
static lv_obj_t *label_estop;

// ==========================
// Create HEGE Control Panel UI
// ==========================
void create_hege_ui()
{
    lv_obj_clean(lv_scr_act());

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "HEGE CONTROL PANEL");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 35);

    lv_obj_t *line = lv_obj_create(lv_scr_act());
    lv_obj_set_size(line, 650, 3);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 95);

    label_can = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_can, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_can, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_can, LV_ALIGN_LEFT_MID, 100, -115);

    label_battery = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_battery, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_battery, LV_ALIGN_LEFT_MID, 100, -70);

    label_drive = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_drive, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_drive, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_drive, LV_ALIGN_LEFT_MID, 100, -25);

    label_steering = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_steering, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_steering, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_steering, LV_ALIGN_LEFT_MID, 100, 20);

    label_speed = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_speed, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_speed, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_speed, LV_ALIGN_LEFT_MID, 100, 65);

    label_estop = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_estop, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_estop, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_estop, LV_ALIGN_LEFT_MID, 100, 110);
}

// ==========================
// Update HEGE UI from global vehicleStatus
// vehicleStatus is defined in VehicleStatus.cpp
// ==========================
void update_hege_ui(lv_timer_t *timer)
{
    (void)timer;

    char buffer[100];

    // canConnected now means HEGE is authenticated / taking control
    snprintf(buffer, sizeof(buffer), "CAN Status: %s",
             vehicleStatus.canConnected ? "Connected" : "ERROR");
    lv_label_set_text(label_can, buffer);

    if (vehicleStatus.batteryValid) {
        snprintf(buffer, sizeof(buffer), "Battery Level: %.1f %%", vehicleStatus.battery);
    } else {
        snprintf(buffer, sizeof(buffer), "Battery Level: NULL");
    }
    lv_label_set_text(label_battery, buffer);

    if (!vehicleStatus.driveModeValid) {
        snprintf(buffer, sizeof(buffer), "Drive Mode: NULL");
    }
    else if (vehicleStatus.canConnected) {
        snprintf(buffer, sizeof(buffer), "Drive Mode: Manual");
    }
    else {
        snprintf(buffer, sizeof(buffer), "Drive Mode: Automatic");
    }
    lv_label_set_text(label_drive, buffer);

    if (vehicleStatus.steeringValid) {
        switch (vehicleStatus.steering) {
            case DIRECTION_FORWARD:
                snprintf(buffer, sizeof(buffer), "Steering: Forward");
                break;

            case DIRECTION_BACKWARD:
                snprintf(buffer, sizeof(buffer), "Steering: Backward");
                break;

            case DIRECTION_LEFT:
                snprintf(buffer, sizeof(buffer), "Steering: Left");
                break;

            case DIRECTION_RIGHT:
                snprintf(buffer, sizeof(buffer), "Steering: Right");
                break;

            default:
                snprintf(buffer, sizeof(buffer), "Steering: NULL");
                break;
        }
    } else {
        snprintf(buffer, sizeof(buffer), "Steering: NULL");
    }
    lv_label_set_text(label_steering, buffer);

    if (vehicleStatus.speedValid) {
    snprintf(buffer, sizeof(buffer), "Speed: L %d | R %d",
             vehicleStatus.leftSpeed,
             vehicleStatus.rightSpeed);
    } else {
    snprintf(buffer, sizeof(buffer), "Speed: L NULL | R NULL");
    }
    lv_label_set_text(label_speed, buffer);

    if (vehicleStatus.estopValid) {
        snprintf(buffer, sizeof(buffer), "Emergency Stop: %s",
                 vehicleStatus.emergencyReleased ? "Released" : "Pressed");
    } else {
        snprintf(buffer, sizeof(buffer), "Emergency Stop: NULL");
    }
    lv_label_set_text(label_estop, buffer);

    lv_obj_invalidate(lv_scr_act());
}