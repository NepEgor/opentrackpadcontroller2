#pragma once

#include <stdint.h>

#define NUM_TRIGGERS 2

void triggers_init();

void triggers_read(uint16_t out[NUM_TRIGGERS]);
