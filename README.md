# HEGE_Control_Panel

## 1. Overview
The HEGE Control Panel is an ESP32-S3-based system for vehicle monitoring and manual control. It receives physical control inputs, communicates with the vehicle via CAN bus, and displays vehicle and control status through an LVGL-based user interface.

Development：PlatformIO

## 2. Hardware
|  GPIO / Expander Pin  |
|----------------|------|
| CAN TX         | 20 |
| CAN RX         | 19 |
| Forward Button | 04 |
| Backward Button| 05 |
| Left Button    | 06 |
| Right Button   | 07 |
| Speed + Button | 08 |
| Speed - Button | 09 |
| ManuelModeSwitch | 12 |

-Baud rate：115200 
-Physical inputs: 4 direction buttons, 2 speed buttons, 1 rotary encoder for MANUAL / AUTO mode selection
-CAN reception remains active in both MANUAL and AUTO mode.

## 3. State mashine
AUTO_MODE ──(mode encoder selects MANUAL /
              send external control request)──► WAIT_EXT_CONTROL_OK(0x215 recieved)
    ▲              (0x195 send)                         │
    │                                                   │
    │                                                   │
    │                  (request denied                  │
    │                      or timeout)                  │
    └───────────────────────────────────────────────────┘
                                                        │
                                      (main system accepted) 
                                                        │
                                                        ▼
                                                MANUAL_ACTIVE(0x295)
                                                        │
                                                        │
                         (mode encoder selects AUTO /
                          send manual release)          │
                                                        │
                                                        ▼
                                                   AUTO_MODE


:CAN messages are always received; Physical input is always read, but physical input is only accepted in MANUAL_ACTIVE.

## 4. CAN Communication
   ### 4.1 Received CAN Messages
   
      0x315  |  recieve speed feedback
      0x215  |  Receive control confirmation / challenge / status feedback, including SOC

**0x215**
| Byte | Content |
| 0    | activation status   
| 1    | random number
| 2    | shift value
| 3    |
| 4    | Feedback of outputs 
| 5    | current gear
| 6-7  | SOC    
**0x315**
 | Byte | Content |
 |0–3	  | speed left	signed |
 |4–7	  | speed right	signed |
   
   ### 4.2 Transmitted CAN Messages

   0x195  |  Send external control request / auth reply / deactivate
   0x295  |  speed control (direktions & speed)

   
**0x295 speed frame layout:**

| Byte | Content |

| 0 | `0x03 \| (dir << 4) \| (dir << 6)` — direction flags | 
| 1–2 | Speed magnitude, little-endian (repeated in bytes 3–4) |
| 5 | `0x01` (active flag) |
| 6–7 | `0x00` |

**byte 0**
|Vehicle movement | dir_left | dir_right |

| Forward         |   `1`    |    `1`    |
| Backward        |   `2`    |    `2`    |
| Left turn       |   `2`    |    `1`    |
| Right turn      |   `1`    |    `2`    |
| Neutral / Stop  |   `0`    |    `0`    |

   
## 5. UI Layout

| UI item          | Source type     | Source             | Description |
|------------------|-----------------|--------------------|-------------|
| Control Source   | Software status | CAN receive check  | Shows `REMOTE CONTROL` when remote control is detected, `PANEL CONTROL` when the control panel has taken control, and `ERROR` when CAN communication is unavailable. |
| Battery        | CAN feedback                         | `0x215`, Byte 6–7              | Displays battery state of charge                                            |
| Speed          | CAN feedback                         | `0x315`, Byte 0–3 and Byte 4–7 | Displays left and right speed feedback                                      |
| Steering mode  | CAN feedback                         | `0x215`, Byte 4                | Displays steering/output feedback; bit mapping still has to be confirmed    |
| Emergency stop | Local physical input                 | Emergency stop button          | Displays whether the local emergency stop is active                         |
| Mode           | Local physical input + state machine | Mode encoder and control state | Displays `AUTO`, `WAIT_EXT_CONTROL_OK`, or `MANUAL_ACTIVE`                  |


## 6.Project Structure
HEGE-Control-Panel/
│
├── main.cpp
│
├── config/
│   └── AppConfig.h
│
├── can/
│   ├── CanDriver.cpp
│   ├── CanDriver.h
│   ├── CanFrame.h
│   ├── CanProtocol.cpp
│   └── CanProtocol.h
│
├── input/
│   ├── InputData.h
│   ├── PhysicalInput.cpp
│   └── PhysicalInput.h
│
├── state/
│   ├── ControlStateMachine.cpp
│   └── ControlStateMachine.h
│
└── ui/
    ├── HEGE_UI.cpp
    ├── HEGE_UI.h
    ├── VehicleStatus.cpp
    └── VehicleStatus.h
    
The project is divided into several functional modules:

main.cpp/ – Initializes the system and coordinates CAN communication, physical inputs, the control state machine, and the user interface.
config/ – Contains the main system configuration parameters.
can/ – Handles CAN driver initialization, CAN frame transmission/reception, and CAN protocol encoding/decoding.
input/ – Handles physical button and mode inputs received from the external ESP32.
state/ – Implements the AUTO/MANUAL control state machine and generates vehicle control commands.
ui/ – Stores vehicle status information and manages the LVGL-based display interface.
External_Input_Board/ – Contains the firmware for the external ESP32 board responsible for reading physical control inputs and transmitting them to the main control panel.


## Important Notes
The repository contains two separate PlatformIO projects:

The root directory contains the firmware for the main HEGE control panel.
The extension/ directory contains the firmware for the external ESP32 input board.

Each board has its own src/ directory and platformio.ini and must therefore be opened and uploaded as a separate PlatformIO project.

Building and Uploading
1.Main Control Panel

To build or upload the firmware for the main control panel:

Clone or download the complete repository.
Open Visual Studio Code with the PlatformIO extension installed.
Select Open Folder and open the repository root:
HEGE_Control_Panel_NEW/
PlatformIO will use the following configuration and source files:
HEGE_Control_Panel_NEW/platformio.ini
HEGE_Control_Panel_NEW/src/
Connect the main control panel to the computer via USB.
Use PlatformIO → Build to compile the project.
Use PlatformIO → Upload to upload the firmware to the main control panel.

2.External Input Board

The external input board must be opened as a separate PlatformIO project.

In Visual Studio Code, select Open Folder.
Open:
HEGE_Control_Panel_NEW/extension/
PlatformIO will then use:
extension/platformio.ini
extension/src/
Connect the external ESP32 board to the computer via USB.
Use PlatformIO → Build to compile the external-board firmware.
Use PlatformIO → Upload to upload the firmware.

Important: Do not build the external input board by opening only the repository root. The main control panel and the external input board are two independent PlatformIO projects and must be opened separately when compiling or uploading their firmware.

