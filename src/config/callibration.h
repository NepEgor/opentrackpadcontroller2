#pragma once

#include <stdint.h>

#include "triggers.h"

struct trigger_callibration_info {
    uint16_t min;
    uint16_t max;
};

struct callibration_info {
    trigger_callibration_info triggers[NUM_TRIGGERS];
};

typedef bool (*bool_callback)();

int callibrate(bool_callback is_callibration_done, bool skip_load = false);

trigger_callibration_info *get_trigger_callibration();
