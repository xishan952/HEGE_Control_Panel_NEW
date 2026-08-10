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

## 3. State Machine


AUTO_MODE ──(mode selector selects MANUAL /
             send manual control request)──────► WAIT_ANSWER_OK
                    (0x150 sent)                      │
                                                      │
                                                      │
                         (mode selector               │
                          returns to AUTO)            │
                                                      │
        ▲                                             │
        │                                             │
        └─────────────────────────────────────────────┘
                                                      │
                                                      │
                              (0x140 received,
                               Byte 0 = 0x01)
                                                      │
                                                      ▼

                                             MANUAL_ACTIVE
                                             (PANEL CONTROL)
                                                      │
                                                      │
                     direction input + preset speed   │
                     send 0x330 periodically          │
                                                      │
                                                      │
                       (mode selector selects AUTO)   │
                                                      │
                                                      ▼

                                                 AUTO_MODE


##Emergency Stop:

ANY STATE
    │
    │  external E-Stop pressed
    │
    ▼
SEND 0x150
Byte 7 = 0xFF
    │
    └── current state remains unchanged

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
   
### 4.2 Sent CAN Messages
0x150 | Send manual control request or emergency stop command
0x330 | Send manual wheel direction and speed command

0x150

Manual control request:

Byte	Content
0	0x02
1-6	0x00
7	0x00
DATA = 02 00 00 00 00 00 00 00

Emergency stop:

Byte	Content
0-6	0x00
7	0xFF
DATA = 00 00 00 00 00 00 00 FF

0x330

Byte	Content
0	left/right wheel direction
1-2	left wheel speed
3-4	right wheel speed
5	gear
6-7	0x00

Direction values:

0 = Neutral
1 = Forward
2 = Backward

Current movement mapping:

Forward    | Left = 1 | Right = 1
Backward   | Left = 2 | Right = 2
Left Turn  | Left = 2 | Right = 1
Right Turn | Left = 1 | Right = 2

Speed range:

0-1000
Default preset speed = 300
SpeedUp   = +100 per press
SpeedDown = -100 per press
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
