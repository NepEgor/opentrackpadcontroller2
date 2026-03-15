#include "ps2.h"
#include "ps2.pio.h"
#include "ps2defines.h"

#include <stdio.h>

PS2::PS2(uint data_pin)
{
    this->data_pin = data_pin;
}

void PS2::begin(PIO pio, uint offset)
{
    this->pio = pio;
    this->offset = offset;

    sm = pio_claim_unused_sm(pio, true);
    printf("claimed pio sm %d\n", sm);

    ps2_program_init(pio, sm, offset, data_pin);
    pio_sm_set_enabled(pio, sm, true);
}

void PS2::restart()
{
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_restart(pio, sm);
    pio_sm_exec_wait_blocking(pio, sm, ps2_wrap_target + offset); // jmp to start
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
    pio_sm_exec_wait_blocking(pio, sm, ps2_offset_write + offset);

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

void PS2::setupDma(uint8_t *packet, uint packet_size)
{
    dma = dma_claim_unused_channel(true);
    printf("pio sm %d, claimed dma %d\n", sm, dma);

    dma_channel_config dma_cfg = dma_channel_get_default_config(dma);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(pio, sm, false));

    // read from the most significant byte
    uint8_t *read_addr = (uint8_t *)&pio->rxf[sm] + 3;

    dma_channel_configure(dma, &dma_cfg,
        packet,
        read_addr,
        packet_size,
        true
    );
}

bool PS2::isPacketReady()
{
    return !dma_channel_is_busy(dma);
}

void PS2::restartDMA(uint8_t *packet)
{
    dma_channel_set_write_addr(dma, packet, true);
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
