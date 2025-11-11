#pragma once

// Command values for Synaptics style queries
const uint8_t ETP_FW_ID_QUERY        = 0x00;
const uint8_t ETP_FW_VERSION_QUERY   = 0x01;
const uint8_t ETP_CAPABILITIES_QUERY = 0x02;
const uint8_t ETP_SAMPLE_QUERY       = 0x03;
const uint8_t ETP_RESOLUTION_QUERY   = 0x04;
const uint8_t ETP_ICBODY_QUERY       = 0x05;

const uint8_t ETP_PS2_CUSTOM_COMMAND = 0xf8;

const uint8_t ETP_REGISTER_READ      = 0x10;
const uint8_t ETP_REGISTER_WRITE     = 0x11;
const uint8_t ETP_REGISTER_READWRITE = 0x00;
