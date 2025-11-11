#include <stdio.h>

#include "elan4.h"
#include "ps2defines.h"
#include "elan4defines.h"

#define ELAN_DEBUG

#ifdef ELAN_DEBUG
#define print(fmt, ...) printf(fmt, ##__VA_ARGS__)
void printParam(uint8_t *param, uint8_t len)
{
    for(uint8_t i = 0; i < len; ++i)
    {
        printf("%02x ", param[i]);
    }
    printf("\n");
}
#else
#define print(fmt, ...)
void inline printParam(uint8_t *param, uint8_t len) {}
#endif

// (value from firmware) * 10 + 790 = dpi
// we also have to convert dpi to dots/mm (*10/254 to avoid floating point)
static unsigned int elantech_convert_res(unsigned int val)
{
	return (val * 10 + 790) * 10 / 254;
}

Elan4::Elan4(uint data_pin) : ps2(data_pin)
{
}

uint8_t Elan4::command(uint8_t command, uint8_t *param)
{
    ps2.writeByte(ETP_PS2_CUSTOM_COMMAND);
    ps2.writeByte(command);
    ps2.command(PS2_CMD_GETINFO, param);

    return 0;
}

void Elan4::writeReg(uint8_t reg, uint8_t val)
{
    ps2.writeByte(ETP_PS2_CUSTOM_COMMAND);
    ps2.writeByte(ETP_REGISTER_READWRITE);
    ps2.writeByte(ETP_PS2_CUSTOM_COMMAND);
    ps2.writeByte(reg);
    ps2.writeByte(ETP_PS2_CUSTOM_COMMAND);
    ps2.writeByte(ETP_REGISTER_READWRITE);
    ps2.writeByte(ETP_PS2_CUSTOM_COMMAND);
    ps2.writeByte(val);
    ps2.writeByte(PS2_CMD_SETSCALE11 & 0xFF);
}

void Elan4::begin(PIO pio, uint sm, uint offset)
{
    ps2.begin(pio, sm, offset);

    uint8_t param[3];

    print("PS2_CMD_RESET_BAT\n");
    ps2.command(PS2_CMD_RESET_BAT, param);
    printParam(param, PS2_RECV_BYTES_COUNT(PS2_CMD_RESET_BAT));

    print("PS2_CMD_GETID\n");
    ps2.command(PS2_CMD_GETID, param);
    printParam(param, PS2_RECV_BYTES_COUNT(PS2_CMD_GETID));

    print("PS2_CMD_RESET_DIS\n");
    ps2.command(PS2_CMD_RESET_DIS);

    elantech_detect();
    elantech_query_info();
    elantech_setup_ps2();

    ps2.command(PS2_CMD_ENABLE);

    ps2.setupDma(packet_dma, packet_size);

    printf("Elan4 ready\n");
}

void Elan4::elantech_detect()
{
    uint8_t param[3];

    print("Elantech magic knock\n");

    ps2.command(PS2_CMD_RESET_DIS);

    ps2.command(PS2_CMD_DISABLE);
    ps2.command(PS2_CMD_SETSCALE11);
    ps2.command(PS2_CMD_SETSCALE11);
    ps2.command(PS2_CMD_SETSCALE11);
    ps2.command(PS2_CMD_GETINFO, param);

    printParam(param, PS2_RECV_BYTES_COUNT(PS2_CMD_GETINFO));

    if (param[0] != 0x3c || param[1] != 0x03 || (param[2] != 0xc8 && param[2] != 0x00))
    {
		print("unexpected magic knock result\n");
    }
}

void Elan4::elantech_query_info()
{
    uint8_t param[3];

    print("Firmware\n");

    ps2.sliced_command(ETP_FW_VERSION_QUERY);
    ps2.command(PS2_CMD_GETINFO, param);
    printParam(param, PS2_RECV_BYTES_COUNT(PS2_CMD_GETINFO));

    //print(elantech_is_signature_valid(param));

    fw_version = (param[0] << 16) | (param[1] << 8) | param[2];
    ic_version = (fw_version & 0x0f0000) >> 16;

    if(fw_version < 0x020030 || fw_version == 0x020600)
    {
		hw_version = 1;
    }
	else
    {
		switch(ic_version)
        {
            case 2:
            case 4:
                hw_version = 2;
                break;
            case 5:
                hw_version = 3;
                break;
            case 6 ... 15:
                // elan 33200v-3600
                hw_version = 4;
                break;
            default:
                hw_version = 0xff;
		}
	}

    crc_enabled = (fw_version & 0x4000) == 0x4000;

    print("fw\t%x\n", fw_version);
    print("hw\t%x\n", hw_version);
    print("ic\t%x\n", ic_version);
    print("crc\t%x\n", crc_enabled);

    print("Capabilities\n");
    command(ETP_CAPABILITIES_QUERY, capabilities);
    printParam(capabilities);

    print("Samples\n");
    command(ETP_SAMPLE_QUERY, samples);
    printParam(samples);

    print("Resolution\n");

    x_res = 31;
	y_res = 31;
	if(hw_version == 4)
    {
        command(ETP_RESOLUTION_QUERY, param);
        printParam(param);

        x_res = elantech_convert_res(param[1] & 0x0f);
        y_res = elantech_convert_res((param[1] & 0xf0) >> 4);
        bus = param[2];
    }

    print("x_res\t%u\n", x_res);
    print("y_res\t%u\n", y_res);
    print("bus\t%u\n", bus);

    // query range information
	switch(hw_version)
    {
        case 3:
            command(ETP_FW_ID_QUERY, param);

            x_max = (0x0f & param[0]) << 8 | param[1];
            y_max = (0xf0 & param[0]) << 4 | param[2];

            break;

        case 4:
            command(ETP_FW_ID_QUERY, param);

            x_max = (0x0f & param[0]) << 8 | param[1];
            y_max = (0xf0 & param[0]) << 4 | param[2];

            // column number of traces
            x_traces = capabilities[1];
            //if ((x_traces < 2) || (x_traces > info->x_max))
            //    return -EINVAL;

            width = x_max / (x_traces - 1);

            // row number of traces
            y_traces = capabilities[2];
            //if ((traces >= 2) && (traces <= info->y_max))
            //    info->y_traces = traces;

            break;

        default: // No plans on supporting hw 1 and 2
            break;
    }

    print("x_max\t%i\n", x_max);
    print("y_max\t%i\n", y_max);
    print("x_traces\t%i\n", x_traces);
    print("y_traces\t%i\n", y_traces);
    print("width\t%i\n", width);
}

void Elan4::elantech_setup_ps2()
{
    print("Set absolute mode - Start\n");

    switch(hw_version)
    {
        case 4:
            reg_07 = 0x01;
            writeReg(0x07, reg_07);

            break;

        default:
            break;
    }

    print("Set absolute mode - Finish\n");
}
