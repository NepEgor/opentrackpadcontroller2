#pragma once

#define PS2_SEND_BYTES_COUNT(COMMAND) ((COMMAND) >> 12) & 0xf
#define PS2_RECV_BYTES_COUNT(COMMAND) ((COMMAND) >> 8) & 0xf

const uint16_t PS2_CMD_SETSCALE11 = 0x00e6;
const uint16_t PS2_CMD_SETSCALE21 = 0x00e7;
const uint16_t PS2_CMD_SETRES     = 0x10e8;
const uint16_t PS2_CMD_GETINFO    = 0x03e9;
const uint16_t PS2_CMD_SETSTREAM  = 0x00ea;
const uint16_t PS2_CMD_SETPOLL    = 0x00f0;
const uint16_t PS2_CMD_POLL       = 0x06eb; // 6 - packet size for elan fw > 1
const uint16_t PS2_CMD_RESET_WRAP = 0x00ec;
const uint16_t PS2_CMD_GETID      = 0x01f2; // 0x02f2 in linux driver; my device returns 1 byte only
const uint16_t PS2_CMD_SETRATE    = 0x10f3;
const uint16_t PS2_CMD_ENABLE     = 0x00f4;
const uint16_t PS2_CMD_DISABLE    = 0x00f5;
const uint16_t PS2_CMD_RESET_DIS  = 0x00f6;
const uint16_t PS2_CMD_RESET_BAT  = 0x02ff;
