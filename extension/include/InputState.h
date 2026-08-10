#pragma once

struct InputState
{
    bool forward = false;
    bool backward = false;

    bool left = false;
    bool right = false;

    bool speedUp = false;
    bool speedDown = false;

    bool estop = false;

    bool manualMode = false;
};