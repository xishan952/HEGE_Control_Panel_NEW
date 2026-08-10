#pragma once

// ==========================
// CAN GPIO
// ==========================

#define CAN_TX_GPIO             20
#define CAN_RX_GPIO             19

// ==========================
// Physical input GPIO
// ==========================

#define BTN_FORWARD_GPIO        4
#define BTN_BACKWARD_GPIO       5
#define BTN_LEFT_GPIO           6
#define BTN_RIGHT_GPIO          7

#define BTN_SPEED_UP_GPIO       8
#define BTN_SPEED_DOWN_GPIO     9

#define ESTOP_GPIO              10

#define MODE_AUTO_GPIO          11
#define MODE_MANUAL_GPIO        12

#define FAULT_LED_GPIO          13

// ==========================
// CAN IDs
// ==========================

// RX: teacher controller -> control panel
#define CAN_ID_CONTROL_140      0x140

// TX: control panel -> teacher controller
#define CAN_ID_CONTROL_150      0x150
#define CAN_ID_SPEED_330        0x330

// Existing vehicle status messages
#define CAN_ID_STATUS_215       0x215
#define CAN_ID_SPEED_315        0x315

// ==========================
// Timing
// ==========================

#define CAN_BITRATE             500000

#define PERIOD_330_MS           20
#define PERIOD_UI_UPDATE_MS     50