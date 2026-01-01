#include "triggers.h"

#include "hardware/adc.h"

const uint8_t pin_trigger[] = {26, 27};
const uint8_t adc_trigger[] = {0, 1}; // pin_trigger - 26

void triggers_init()
{
    adc_init();
    for (uint8_t i = 0; i < sizeof(pin_trigger); ++i)
    {
        adc_gpio_init(pin_trigger[i]);
    }
}

void triggers_read(uint16_t out[NUM_TRIGGERS])
{
    for (uint8_t i = 0; i < NUM_TRIGGERS; ++i)
    {
        adc_select_input(adc_trigger[i]);
        out[i] = adc_read();
    }
}
