#pragma once

#include "ps2.pio.h"
#include <hardware/dma.h>

class PS2
{
private:
    PIO pio;
    uint sm;
    uint offset;

    uint data_pin;

public:
    // clock_pin = data_pin + 1
    PS2(uint data_pin);

    void begin(PIO pio, uint sm, uint offset);
    void restart();

    bool readByte(uint8_t &data);
    uint8_t readByteBlocking();
    uint8_t writeByte(uint8_t data);

    void setupDma(uint8_t *packet, uint packet_size);
    bool isPacketReady();
    void restartDMA(uint8_t *packet);

    // write command byte
    // command format 0xARCC
    // A - number of args
    // R - number of returns
    // CC - command
    // command args and returns in params array
    uint8_t command(uint16_t command, uint8_t *param = NULL);

    // sliced_command() sends an extended PS/2 command to the mouse
    // using sliced syntax, understood by advanced devices, such as Logitech
    // or Synaptics touchpads. The command is encoded as:
    // 0xE6 0xE8 rr 0xE8 ss 0xE8 tt 0xE8 uu where (rr*64)+(ss*16)+(tt*4)+uu
    // is the command.
    uint8_t sliced_command(uint8_t command);
};
