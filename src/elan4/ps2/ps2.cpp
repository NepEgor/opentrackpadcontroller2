#include "ps2.h"
#include "ps2.pio.h"
#include "ps2defines.h"

PS2::PS2(uint data_pin)
{
    this->data_pin = data_pin;
}

void PS2::begin()
{
    pio_claim_free_sm_and_add_program(&ps2_program, &pio, &sm, &offset);
    ps2_program_init(pio, sm, offset, data_pin);
    pio_sm_set_enabled(pio, sm, true);
}

void PS2::restart()
{
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_restart(pio, sm);
    pio_sm_exec_wait_blocking(pio, 0, ps2_wrap_target + offset); // jmp to start
}

bool PS2::readByte(uint8_t &data)
{
    if (pio_sm_is_rx_fifo_empty(pio, sm))
    {
        return false;
    }

    data = (pio_sm_get(pio, sm) >> 24) & 0xFF;
    return true;
}

uint8_t PS2::readByteBlocking()
{
    return (pio_sm_get_blocking(pio, sm) >> 24) & 0xFF;
}

uint8_t PS2::writeByte(uint8_t data)
{
    pio_sm_put_blocking(pio, sm, ps2_make_frame(data));
    pio_sm_exec_wait_blocking(pio, 0, ps2_offset_write + offset);

    data = readByteBlocking();

    switch (data)
    {
        case 0xFA: // ACK
            return 0;
    
        case 0xFC: // ERROR
            return 1;

        case 0xFE: // NAC
            return 2;

        default:
            return -1;
    }
}

uint8_t PS2::command(uint16_t command, uint8_t *param)
{
    if (writeByte(command & 0xFF))
    {
        return 1;
    }

    uint8_t N = PS2_SEND_BYTES_COUNT(command);
    for (uint8_t i = 0; i < N; ++i)
    {
        if (writeByte(param[i]))
        {
            return 2;
        }
    }

    N = PS2_RECV_BYTES_COUNT(command);
    for (uint8_t i = 0; i < N; ++i)
    {
        param[i] = readByteBlocking();
    }

    return 0;
}

uint8_t PS2::sliced_command(uint8_t command)
{
    writeByte(PS2_CMD_SETSCALE11 & 0xFF);

    for (int8_t shift = 6; shift >= 0; shift -= 2)
    {
        writeByte(PS2_CMD_SETRES & 0xFF);
        writeByte((command >> shift) & 0b11);
    }

    return 0;
}
