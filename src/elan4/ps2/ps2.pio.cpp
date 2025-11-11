#include "ps2.pio.h"
#include "hardware/clocks.h"

void ps2_program_init(PIO pio, uint sm, uint offset, uint data_pin)
{
    pio_sm_config config = ps2_program_get_default_config(offset);

    // set clock to 100kHz
    float div = (float)clock_get_hz(clk_sys) / (100000);
    sm_config_set_clkdiv(&config, div);

    uint clock_pin = data_pin + 1;

    pio_gpio_init(pio, data_pin);
    pio_gpio_init(pio, clock_pin);

    gpio_pull_up(data_pin);
    gpio_pull_up(clock_pin);

    pio_sm_set_consecutive_pindirs(pio, sm, data_pin, 2, false); // set clock and data pins as input

    sm_config_set_in_pins(&config, data_pin);    
    sm_config_set_in_shift(&config, true, true, 8);

    sm_config_set_out_pins(&config, data_pin, 1);
    sm_config_set_out_shift(&config, true, true, 10); // 1 byte + parity bit + stop bit

    sm_config_set_set_pins(&config, data_pin, 2);

    pio_sm_init(pio, sm, offset, &config);
}

uint16_t ps2_make_frame(uint8_t data)
{
    uint16_t parity = data ^ data >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return 0x200 | (~parity & 1) << 8 | data;
}
