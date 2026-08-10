# HEGE_Control_Panel

## 1. Overview

## 2. Hardware
|  GPIO / Expander Pin  |
|----------------|------|
| CAN TX         | TODO |
| CAN RX         | TODO |
| Forward Button | TODO |
| Backward Button| TODO |
| Left Button    | TODO |
| Right Button   | TODO |
| Speed + Button |      |
| Speed - Button |      |
| Mode 1         |      |
| Mode 2         |      |

-CAN bus: TODO kbit/s
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
   不受到state maschine的控制，持续接收can 信号
   
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

   ### 4.3 CAN Timeout
   
## 5. UI Layout

| UI item        | Source type                          | Source                         | Description                                                                 |
| -------------- | ------------------------------------ | ------------------------------ | --------------------------------------------------------------------------- |
| CAN connected  | Software status                      | CAN receive check              | Shows `Connected` if valid CAN frames are received; otherwise shows `Error` |
| Battery        | CAN feedback                         | `0x215`, Byte 6–7              | Displays battery state of charge                                            |
| Speed          | CAN feedback                         | `0x315`, Byte 0–3 and Byte 4–7 | Displays left and right speed feedback                                      |
| Steering mode  | CAN feedback                         | `0x215`, Byte 4                | Displays steering/output feedback; bit mapping still has to be confirmed    |
| Emergency stop | Local physical input                 | Emergency stop button          | Displays whether the local emergency stop is active                         |
| Mode           | Local physical input + state machine | Mode encoder and control state | Displays `AUTO`, `WAIT_EXT_CONTROL_OK`, or `MANUAL_ACTIVE`                  |



## 6. Physical Inputs
## 7. Timing
## 8. Safety and Error Handling
## 9. Power Supply
## 10. Mechanical Design
## 11. Main Loop Structure
## 12. Software Structure
## 13. Testing
## 14. Notes
