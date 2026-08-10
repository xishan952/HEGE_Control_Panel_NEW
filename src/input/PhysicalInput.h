#pragma once

#include "InputData.h"

// Initialize all physical input GPIOs
void physical_input_init();

// Read current physical input state
PhysicalInput read_physical_input();