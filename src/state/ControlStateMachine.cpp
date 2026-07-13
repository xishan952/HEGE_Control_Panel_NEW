#include "ControlStateMachine.h"

#include "../can/CanDriver.h"
#include "../can/CanProtocol.h"
#include "../ui/VehicleStatus.h"
#include "../config/AppConfig.h"

// ================================================================
// Internal state
// ================================================================

static ControlState controlState =
    ControlState::AUTO_MODE;

static unsigned long lastSpeedCommandMs = 0;

// Default manual speed setting
static const uint16_t DEFAULT_SPEED = 300;

// Speed adjustment step
static const uint16_t SPEED_STEP = 100;

// Current preset speed used when a direction button is pressed
static uint16_t currentSpeedMagnitude =
    DEFAULT_SPEED;

// Previous button states for edge detection
static bool previousSpeedUp = false;
static bool previousSpeedDown = false;

// ================================================================
// Helper: build command from physical input
// ================================================================

static ManualControlCommand buildManualCommand(
    const PhysicalInput& input
)
{
    ManualControlCommand command;

    command.speedMagnitude =
        currentSpeedMagnitude;

    command.gear = 1;

    if (input.forward) {
        command.movement =
            MovementCommand::FORWARD;
    }
    else if (input.backward) {
        command.movement =
            MovementCommand::BACKWARD;
    }
    else if (input.left) {
        command.movement =
            MovementCommand::LEFT_TURN;
    }
    else if (input.right) {
        command.movement =
            MovementCommand::RIGHT_TURN;
    }
    else {
        command.movement =
            MovementCommand::NEUTRAL;
    }

    return command;
}

// ================================================================
// Initialize state machine
// ================================================================

void control_state_init()
{
    controlState =
        ControlState::AUTO_MODE;

    controlAnswerOk = false;

    currentSpeedMagnitude =
        DEFAULT_SPEED;

    previousSpeedUp = false;
    previousSpeedDown = false;

    lastSpeedCommandMs = 0;

    vehicleStatus.driveModeValid = true;
    vehicleStatus.manualMode = false;

    vehicleStatus.estopCommandSent = false;

    vehicleStatus.controlSource =
        CONTROL_SOURCE_ERROR;
}

// ================================================================
// Update state machine
// ================================================================

void control_state_update(
    const PhysicalInput& input
)
{
    unsigned long now = millis();

    // ============================================================
    // Update E-Stop UI status from physical input
    // ============================================================

    vehicleStatus.estopCommandSent =
        input.estop;

    // ============================================================
    // Emergency stop has highest priority
    // ============================================================

    if (input.estop) {

        CanFrame estopFrame =
            buildEstop150();

        can_driver_send(estopFrame);

        return;
    }

    // ============================================================
    // State machine
    // ============================================================

    switch (controlState) {

        // ========================================================
        // AUTO_MODE
        // ========================================================

        case ControlState::AUTO_MODE:

            vehicleStatus.driveModeValid = true;
            vehicleStatus.manualMode = false;

            if (input.selectedMode ==
                PanelMode::MANUAL) {

                controlAnswerOk = false;

                CanFrame requestFrame =
                    buildControlRequest150();

                can_driver_send(requestFrame);

                controlState =
                    ControlState::WAIT_ANSWER_OK;
            }

            break;

        // ========================================================
        // WAIT_ANSWER_OK
        // ========================================================

        case ControlState::WAIT_ANSWER_OK:

            vehicleStatus.driveModeValid = true;
            vehicleStatus.manualMode = false;

            // User changed back to AUTO
            if (input.selectedMode ==
                PanelMode::AUTO) {

                controlAnswerOk = false;

                controlState =
                    ControlState::AUTO_MODE;

                break;
            }

            // 0x140 Byte 0 == 0x01 received
            if (controlAnswerOk) {

                controlAnswerOk = false;

                controlState =
                    ControlState::MANUAL_ACTIVE;

                vehicleStatus.manualMode = true;

                // Panel has successfully taken control
                vehicleStatus.controlSource =
                    CONTROL_SOURCE_PANEL;

                // Reset preset speed whenever manual control
                // becomes active
                currentSpeedMagnitude =
                    DEFAULT_SPEED;

                previousSpeedUp = false;
                previousSpeedDown = false;

                lastSpeedCommandMs = 0;
            }

            break;

        // ========================================================
        // MANUAL_ACTIVE
        // ========================================================

        case ControlState::MANUAL_ACTIVE:

            vehicleStatus.driveModeValid = true;
            vehicleStatus.manualMode = true;

            // Keep PANEL CONTROL as highest priority
            vehicleStatus.controlSource =
                CONTROL_SOURCE_PANEL;

            // Return to AUTO mode
            if (input.selectedMode ==
                PanelMode::AUTO) {

                currentSpeedMagnitude =
                    DEFAULT_SPEED;

                previousSpeedUp = false;
                previousSpeedDown = false;

                controlState =
                    ControlState::AUTO_MODE;

                vehicleStatus.manualMode = false;

                // Do not force REMOTE here.
                // The next valid 0x215 with Byte 0 == 0x03
                // will set CONTROL_SOURCE_REMOTE.

                break;
            }

            // ----------------------------------------------------
            // Speed button edge detection
            // One press = one speed step
            // ----------------------------------------------------

            {
                bool speedUpPressed =
                    input.speedUp &&
                    !previousSpeedUp;

                bool speedDownPressed =
                    input.speedDown &&
                    !previousSpeedDown;

                previousSpeedUp =
                    input.speedUp;

                previousSpeedDown =
                    input.speedDown;

                // ------------------------------------------------
                // Speed adjustment
                // Range: 0 ... 1000
                // Step: 100
                // ------------------------------------------------

                if (speedUpPressed) {

                    if (currentSpeedMagnitude <=
                        1000 - SPEED_STEP) {

                        currentSpeedMagnitude +=
                            SPEED_STEP;
                    }
                    else {
                        currentSpeedMagnitude = 1000;
                    }
                }

                if (speedDownPressed) {

                    if (currentSpeedMagnitude >=
                        SPEED_STEP) {

                        currentSpeedMagnitude -=
                            SPEED_STEP;
                    }
                    else {
                        currentSpeedMagnitude = 0;
                    }
                }
            }

            // ----------------------------------------------------
            // Send 0x330 periodically
            // ----------------------------------------------------

            if (now - lastSpeedCommandMs >=
                PERIOD_330_MS) {

                lastSpeedCommandMs = now;

                ManualControlCommand command =
                    buildManualCommand(input);

                CanFrame speedFrame =
                    buildSpeedCommand330(command);

                can_driver_send(speedFrame);
            }

            break;
    }
}

// ================================================================
// Get current state
// ================================================================

ControlState get_control_state()
{
    return controlState;
}