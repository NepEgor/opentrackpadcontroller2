#include "gyro.h"

#include <stdint.h>

#include "util_func.h"

Gyro::Gyro()
{
    invert_x = 1;
    invert_y = 1;
    invert_z = 1;

    sensitivity = 1.0f;

    time0 = 0;
    delay = 0;

    bind_to_x = BIND_X;

    _EnabledCallback = [] { return false; };
}

void Gyro::init()
{
    mpu.initialize();

    /*
    mpu.setXGyroOffset(0);
    mpu.setYGyroOffset(0);
    mpu.setZGyroOffset(0);
    */
    mpu.CalibrateGyro(6);

    //mpu.setIntDataReadyEnabled(1);

    //mpu.setDLPFMode(MPU6050_DLPF_BW_5);
}

void Gyro::update(uint32_t time)
{
    if (Enabled())
    {
        float dt = time - time0;
        if (dt > delay)
        {
            time0 = time;

            int16_t x, y, z;

            mpu.getRotation(&x, &y, &z);

            dt /= 1000.0f;

            this->x = x * sensitivity * invert_x * dt;
            this->y = y * sensitivity * invert_y * dt;
            this->z = z * sensitivity * invert_z * dt;
        }
    }
}

int16_t Gyro::getDX()
{
    int32_t dx;

    switch (bind_to_x)
    {
        case BIND_X:
            dx = x;
            break;

        case BIND_Z:
            dx = z;
            break;

        case BIND_XZ:
            dx = x + z;
    }
    
    if (dx > -deadzone && dx < deadzone)
    {
        dx = 0;
    }
    else
    if (dx >= deadzone && dx < min_delta)
    {
        dx = min_delta;
    }
    else
    if (dx <= -deadzone && dx > -min_delta)
    {
        dx = -min_delta;
    }
    
    return clamp(dx, -32767, 32767);
}

int16_t Gyro::getDY()
{
    
    if (y > -deadzone && y < deadzone)
    {
        y = 0;
    }
    else
    if (y >= deadzone && y < min_delta)
    {
        y = min_delta;
    }
    else
    if (y <= -deadzone && y > -min_delta)
    {
        y = -min_delta;
    }
    
    return clamp(y, -32767, 32767);
}
