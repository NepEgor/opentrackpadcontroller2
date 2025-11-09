#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "elan4.h"
#include "gyro.h"
#include "usb_device.h"

Elan4 trackpad(27);
Gyro gyro;

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

    i2c_init(i2c_default, 400*1000);
    
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    gyro.init();
    gyro.setEnabledCallback([] { return true; });
    gyro.enable();
    gyro.setInvertY();
    gyro.setSensitivity(1.0f);
    gyro.setDeadzone(0);
    gyro.setMinDelta(1000);
    gyro.setBindToX(Gyro::BIND_XZ);
    gyro.setDelay(1000);

    //trackpad.begin();

    //uint8_t tevent_size;
    //TouchEvent tevent[5];

    //XInputReport report = {0};
    //report.report_size = XINPUT_REPORT_SIZE;

    uint8_t button_old = 0;

    xinput.begin();

    while (1)
    {
        //tevent_size = trackpad.poll(tevent);

        uint8_t button = gpio_get(23);
        gpio_put(25, button);

        if (button_old != button)
        {
            xinput.button(0x01, button);
            
            button_old = button;
        }

        gyro.update(time_us_32());
        int32_t x = gyro.getX();
        int32_t y = gyro.getY();
        
        xinput.joystick(0, x, y);

        xinput.sendReport();
        //xinput.recvReport(); todo doesn't work

        xinput.tudTask();
    }
}
