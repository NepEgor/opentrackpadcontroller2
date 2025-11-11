#include <stdio.h>
#include <string.h>

#include "elan4.h"

enum PacketType: uint8_t
{
    PACKET_UNKNOWN    = 0x01,
    PACKET_DEBOUNCE   = 0x02,
    PACKET_V3_HEAD    = 0x03,
    PACKET_V3_TAIL    = 0x04,
    PACKET_V4_HEAD    = 0x05,
    PACKET_V4_MOTION  = 0x06,
    PACKET_V4_STATUS  = 0x07,
    PACKET_TRACKPOINT = 0x08,
};

uint8_t Elan4::elantech_packet_check_v4()
{
    packet_type = packet[3] & 0x03;

    bool sanity_check = (packet[0] & 0x08) == 0x00 && (packet[3] & 0x1c) == 0x10;
    if (!sanity_check)
    {
        packet_type = PACKET_UNKNOWN;
    }
    else
    {
        switch (packet_type)
        {
            case 0:
                packet_type = PACKET_V4_STATUS;
                break;

            case 1:
                packet_type = PACKET_V4_HEAD;
                break;

            case 2:
                packet_type = PACKET_V4_MOTION;
                break;

            default:
                packet_type = PACKET_UNKNOWN;
                break;
        }
    }

    return packet_type;
}

uint8_t Elan4::process_packet_status_v4(TouchEvent* tevent)
{
    //printf("Status\n");

    touching_prev = touching;
    touching = packet[1] & 0x1f;

    uint8_t size = 0;

    uint8_t dif01 = (touching ^ touching_prev) & ~touching_prev; // sets changes from 0 to 1
    uint8_t dif10 = (touching ^ touching_prev) & ~touching;      // sets changes from 1 to 0

    //printf("S %X %X %X %X\n", touching, touching_prev, dif01, dif10);

    if (dif01)
    {
        for (uint8_t i = 0; i < fingers_num; ++i)
        {
            if(dif01 & 1)
            {
                fingers[i].dx = 0;
                fingers[i].dy = 0;
            }

            dif01 >>= 1;
        }
    }

    if (dif10)
    {
        for (uint8_t i = 0; i < fingers_num; ++i)
        {
            if(dif10 & 1)
            {
                tevent[size].finger_id = i;
                tevent[size].type = TET_UP;
                ++size;
            }

            dif10 >>= 1;
        }
    }

    return size;
}

uint8_t Elan4::process_packet_head_v4(TouchEvent* tevent)
{
    //printf("Head\n");

    int8_t fid = ((packet[3] & 0xe0) >> 5) - 1;
    if(fid < 0) return 0;

    uint8_t size = 1;

    uint8_t pres, traces;

    int32_t x = ((packet[1] & 0x0f) << 8) | packet[2];
    int32_t y = ((packet[4] & 0x0f) << 8) | packet[5];

    // if finger just touched down
    if (((touching ^ touching_prev) & (1 << fid)))
    {
        fingers[fid].dx = 0;
        fingers[fid].dy = 0;

        touching_prev ^= touching;

        tevent[0].type = TET_DOWN;
    }
    // if finger has been touching
    else
    {
        fingers[fid].dx = fingers[fid].x - x;
        fingers[fid].dy = fingers[fid].y - y;

        tevent[0].type = TET_MOVE;
    }

    fingers[fid].x = x;
    fingers[fid].y = y;

    pres = (packet[1] & 0xf0) | ((packet[4] & 0xf0) >> 4);
    traces = (packet[0] & 0xf0) >> 4;

    tevent[0].finger_id = fid;
    tevent[0].fp.x = x;
    tevent[0].fp.y = y;
    tevent[0].fp.dx = fingers[fid].dx;
    tevent[0].fp.dy = fingers[fid].dy;

    //printf("H f %i  x %i  y %i\n", fid, fingers[fid].x, fingers[fid].y);

    return size;
}

uint8_t Elan4::process_packet_motion_v4(TouchEvent* tevent)
{
    //printf("Motion\n");

    int16_t fid = ((packet[0] & 0xe0) >> 5) - 1;
    if (fid < 0) return 0;

    int16_t sid = ((packet[3] & 0xe0) >> 5) - 1;
    int16_t weight = (packet[0] & 0x10) ? 5 : 1;

    int32_t delta_x1 = (int8_t)packet[1];
    int32_t delta_y1 = (int8_t)packet[2];
    int32_t delta_x2 = (int8_t)packet[4];
    int32_t delta_y2 = (int8_t)packet[5];

    fingers[fid].dx = delta_x1 * weight;
    fingers[fid].dy = delta_y1 * weight;

    fingers[fid].x += fingers[fid].dx;
    fingers[fid].y += fingers[fid].dy;

    uint8_t size = 1;
    tevent[0].finger_id = fid;
    tevent[0].type = TET_MOVE;
    tevent[0].fp.x = fingers[fid].x;
    tevent[0].fp.y = fingers[fid].y;
    tevent[0].fp.dx = fingers[fid].dx;
    tevent[0].fp.dy = fingers[fid].dy;

    //printf("M f %i  x %i  y %i\n", fid, fingers[fid].x, fingers[fid].y);

    if(sid >= 0)
    {
        fingers[sid].dx = delta_x2 * weight;
        fingers[sid].dy = delta_y2 * weight;

        fingers[sid].x += fingers[sid].dx;
        fingers[sid].y += fingers[sid].dy;
    
        size = 2;
        tevent[1].finger_id = sid;
        tevent[1].type = TET_MOVE;
        tevent[1].fp.x = fingers[sid].x;
        tevent[1].fp.y = fingers[sid].y;
        tevent[1].fp.dx = fingers[sid].dx;
        tevent[1].fp.dy = fingers[sid].dy;

        //printf("  f %i  x %i  y %i\n", sid, fingers[sid].x, fingers[sid].y);
    }

    return size;
}

int8_t Elan4::poll(TouchEvent* tevent)
{
    if (!ps2.isPacketReady())
    {
        return -1; // no data available
    }

    memcpy(packet, packet_dma, packet_size);
    ps2.restartDMA(packet_dma);

    elantech_packet_check_v4();
    switch(packet_type) {
        case PACKET_V4_STATUS:
            return process_packet_status_v4(tevent);

        case PACKET_V4_HEAD:
            return process_packet_head_v4(tevent);

        case PACKET_V4_MOTION:
            return process_packet_motion_v4(tevent);
        
        case PACKET_UNKNOWN:
            printf("PACKET_UNKNOWN\n");
            return -2;
        
        case PACKET_TRACKPOINT:
            printf("PACKET_TRACKPOINT\n");
            return -2;

        default:
            return -3;
    }
}
