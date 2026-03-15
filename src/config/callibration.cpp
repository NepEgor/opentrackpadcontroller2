#include "callibration.h"

#include "ff.h"

#include <stdio.h>

const char *callibration_file = "callibration.bin";

callibration_info callibration;

void callibration_print()
{
    printf("trigger callibration:\n");

    for (uint8_t i = 0; i < NUM_TRIGGERS; i++)
    {
        printf("trigger %d min %d max %d\n", i, callibration.triggers[i].min, callibration.triggers[i].max);
    }
}

int callibration_load()
{
    FRESULT fr;
    FIL fil;

    fr = f_open(&fil, callibration_file, FA_READ);
    if (FR_OK != fr)
    {
        return -1;
    }

    UINT bytes_read;
    fr = f_read(&fil, &callibration, sizeof(callibration), &bytes_read);
    if (FR_OK != fr)
    {
        return -1;
    }

    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        return -1;
    }

    callibration_print();

    return 0;
}

int callibration_save()
{
    FRESULT fr;
    FIL fil;

    fr = f_open(&fil, callibration_file, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fr)
    {
        return -1;
    }

    UINT bytes_written;
    fr = f_write(&fil, &callibration, sizeof(callibration), &bytes_written);
    if (FR_OK != fr)
    {
        return -1;
    }

    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        return -1;
    }

    return 0;
}

void callibrate_triggers()
{
    uint16_t triggers[NUM_TRIGGERS];
    triggers_read(triggers);

    for (uint8_t i = 0; i < NUM_TRIGGERS; i++)
    {
        if (triggers[i] < callibration.triggers[i].min)
        {
            callibration.triggers[i].min = triggers[i];
        }
        if (triggers[i] > callibration.triggers[i].max)
        {
            callibration.triggers[i].max = triggers[i];
        }
    }
}

void callibration_loop(bool_callback is_callibration_done)
{
    while (!is_callibration_done())
    {
        callibrate_triggers();
    }
}

void callibration_init()
{
    for (uint8_t i = 0; i < NUM_TRIGGERS; i++)
    {
        callibration.triggers[i].min = UINT16_MAX;
        callibration.triggers[i].max = 0;
    }
}

int callibrate(bool_callback is_callibration_done, bool skip_load)
{
    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr)
    {
        printf("f_mount error: %d\n", fr);
        skip_load = true;
    }

    int loaded = -1;
    if (!skip_load)
    {
        printf("loading callibration\n");
        loaded = callibration_load();
    }
    else
    {
        printf("skipping callibration load\n");
    }

    if (loaded != 0)
    {
        printf("callibrating\n");
        callibration_init();
        callibration_loop(is_callibration_done);
        callibration_print();
        callibration_save();
    }

    f_unmount("");

    return 0;
}

trigger_callibration_info *get_trigger_callibration()
{
    return callibration.triggers;
}
