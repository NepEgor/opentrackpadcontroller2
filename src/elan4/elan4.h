#pragma once

#include "ps2.h"

void printParam(uint8_t *param, uint8_t len = 3);

struct FingerPosition
{
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
};

enum TouchEventType : uint8_t
{
    TET_DOWN,
    TET_MOVE,
    TET_UP,
};

struct TouchEvent
{
    TouchEventType type;
    uint8_t finger_id;
    FingerPosition fp;
};

class Elan4
{
private:
    PS2 ps2;

    uint8_t  hw_version;
    uint32_t fw_version;
    uint8_t  ic_version;
    bool crc_enabled;

    uint8_t capabilities[3];
    uint8_t samples[3];

    uint32_t x_res;
    uint32_t y_res;
    uint32_t bus;

    int32_t x_max;
    int32_t y_max;

    uint8_t x_traces;
    uint8_t y_traces;
    uint8_t width;

    uint8_t reg_07;
    uint8_t reg_10;

    static const uint8_t packet_size = 6;
    uint8_t packet_type;
    uint8_t packet[packet_size];
    uint8_t packet_dma[packet_size];

public:
    static const uint8_t fingers_num = 5;
private:
    uint8_t touching_prev;
    uint8_t touching;
    FingerPosition fingers[fingers_num];

    uint8_t command(uint8_t command, uint8_t *param);
    void writeReg(uint8_t reg, uint8_t val);

    void elantech_detect();
    void elantech_query_info();
    void elantech_setup_ps2();

    uint8_t elantech_packet_check_v4();
    uint8_t process_packet_status_v4(TouchEvent* tevent);
    uint8_t process_packet_head_v4(TouchEvent* tevent);
    uint8_t process_packet_motion_v4(TouchEvent* tevent);

public:
    Elan4(uint data_pin);

    void begin(PIO pio, uint sm, uint offset);

    int8_t poll(TouchEvent* tevent);

    int32_t getMaxX() { return x_max; }
    int32_t getMaxY() { return y_max; }
};
