#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#include "elan4.h"
#include "input_mapper.h"
#include "usb_device.h"

const uint8_t pin_led = PICO_DEFAULT_LED_PIN; // 25
const uint8_t pin_key_button = 23;

const uint8_t pin_trigger[] = {26, 27};
const uint8_t adc_trigger[] = {0, 1}; // pin_trigger - 26

const uint8_t pin_button[] = {
    19,  // START
    18,  // SELECT
    6,  // BUMPER_LEFT
    7,  // BUMPER_RIGHT
    10, // HOME
    2,  // GRIP_A
    3,  // GRIP_B
    8,  // GRIP_X
    9,  // GRIP_Y
    28, // TRACKPAD_LEFT
    29, // TRACKPAD_RIGHT
};

uint8_t button_state[sizeof(pin_button)] = {0};

const uint8_t pin_gyro_sda = PICO_DEFAULT_I2C_SDA_PIN; // 4
const uint8_t pin_gyro_scl = PICO_DEFAULT_I2C_SCL_PIN; // 5

#ifndef DISABLE_TRACKPADS

//const uint8_t pin_trackpad_data[] = {14, 16}; // elan 33200v-3600 pad T8
//const uint8_t pin_trackpad_clock[] = {15, 17}; // elan 33200v-3600 pad T7

Elan4 trackpad[] = {
    Elan4(14),
    Elan4(16)
};

const uint8_t trackpad_count = sizeof(trackpad) / sizeof(trackpad[0]);

int32_t trackpad_maxX, trackpad_maxY;

#endif

int main()
{
    stdio_init_all();

    gpio_init(pin_led); // led
    gpio_set_dir(pin_led, GPIO_OUT);
    gpio_put(pin_led, 1);

    gpio_init(pin_key_button);
    gpio_set_dir(pin_key_button, GPIO_IN);
    gpio_pull_up(pin_key_button);

    i2c_init(i2c_default, 400*1000);
    
    gpio_set_function(pin_gyro_sda, GPIO_FUNC_I2C);
    gpio_set_function(pin_gyro_scl, GPIO_FUNC_I2C);
    gpio_pull_up(pin_gyro_sda);
    gpio_pull_up(pin_gyro_scl);

    adc_init();
    for (uint8_t i = 0; i < sizeof(pin_trigger); ++i)
    {
        adc_gpio_init(pin_trigger[i]);
    }

    for (uint8_t i = 0; i < sizeof(pin_button); ++i)
    {
        gpio_init(pin_button[i]);
        gpio_set_dir(pin_button[i], GPIO_IN);
        gpio_pull_down(pin_button[i]);
    }
    
#ifndef DISABLE_TRACKPADS

    for (uint8_t i = 0; i < trackpad_count; ++i)
    {
        trackpad[i].begin();
    }

    trackpad_maxX = trackpad[0].getMaxX();
    trackpad_maxY = trackpad[0].getMaxY();

#endif

    InputMapper::begin();

    gpio_put(pin_led, 0);

    int8_t tevent_size;
    TouchEvent tevent[5];

    while (1)
    {
        uint32_t timestamp = time_us_32();

#ifndef DISABLE_TRACKPADS
        for (uint8_t t = 0; t < trackpad_count; ++t)
        {
            tevent_size = trackpad[t].poll(tevent);

            if (tevent_size > 0)
            {
                for (uint8_t i = 0; i < tevent_size; ++i)
                {
                    int32_t x = -1;
                    int32_t y = -1;
                    int32_t dx = 0;
                    int32_t dy = 0;
                    switch (tevent[i].type)
                    {
                        case TET_DOWN:
                        case TET_MOVE:
                            // trackpad is rotated 90 deg so x and y are switched
                            x = tevent[i].fp.y;
                            y = tevent[i].fp.x;
                            dx = tevent[i].fp.dy;
                            dy = tevent[i].fp.dx;
                            
                            // invert axis for the trackpads
                            if(t == 0)
                            {
                                y = trackpad_maxX - y;
                                dy = -dy;
                            }
                            else
                            {
                                x = trackpad_maxY - x;
                                dx = -dx;
                            }

                            break;
                        
                        case TET_UP:
                            break;

                        default:
                            break;
                    }
                    
                    InputMapper::mapTrackpad(t, tevent[i].finger_id, x, y, dx, dy, timestamp);
                }
            }
        }
#endif

        uint16_t triggers[2];
        for (uint8_t i = 0; i < sizeof(pin_trigger); ++i)
        {
            triggers[i] = adc_read();
        }

        //Serial.print(0);
        //Serial.print('\t');
        //Serial.print(triggers[0]);
        //Serial.print('\t');
        //Serial.print(triggers[1]);
        //Serial.print('\t'); 
        //Serial.println(1000);

        InputMapper::mapTriggers(triggers);

        for (uint8_t i = 0; i < sizeof(pin_button); ++i)
        {
            uint8_t value = gpio_get(pin_button[i]);
            if (value != button_state[i])
            {
                if (InputMapper::mapButton((InputMapper::HardwareButtons)(i), value))
                {
                    button_state[i] = value;
                }
            }
        }

        InputMapper::update(timestamp);

        InputMapper::sendReport();

        InputMapper::tudTask();
    }
}
