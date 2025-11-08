#include <stdint.h>
#include "pico/stdlib.h"

#include "elan4.h"
#include "usb_device.h"

Elan4 trackpad(27);
USB_Device xinput;

int main()
{
    stdio_init_all();

    gpio_init(25); // led
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1);

    gpio_init(23); // button
    gpio_set_dir(23, GPIO_IN);
    gpio_pull_up(23);

    xinput.begin();

    //trackpad.begin();

    //uint8_t tevent_size;
    //TouchEvent tevent[5];

    //XInputReport report = {0};
    //report.report_size = XINPUT_REPORT_SIZE;

    uint8_t button_old = 0;

    while (1)
    {
        //tevent_size = trackpad.poll(tevent);

        uint8_t button = gpio_get(23);
        gpio_put(25, button);

        if (button_old != button)
        {
            xinput.button(0x01, button);
            xinput.sendReport();
            button_old = button;
        }

        //xinput.recvReport(); todo doesn't work

        xinput.tudTask();
    }
}
