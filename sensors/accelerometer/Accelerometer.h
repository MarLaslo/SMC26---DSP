#pragma once

#include "daisy_seed.h"

using namespace daisy;

// Adafruit MMA8451 accelerometer on I2C1 (SCL = D11, SDA = D12).
// Based on the Arduino Adafruit_MMA8451 library. Range: +-2g, 14-bit.
class Accelerometer
{
  public:
    // Acceleration in m/s^2, filled in by Read()
    float x, y, z;

    // Returns false if the sensor is not found
    bool Init()
    {
        I2CHandle::Config cfg;
        cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
        cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        cfg.mode           = I2CHandle::Config::Mode::I2C_MASTER;
        cfg.pin_config.scl = seed::D11;
        cfg.pin_config.sda = seed::D12;
        i2c_.Init(cfg);

        if(ReadReg(0x0D) != 0x1A) // WHO_AM_I
            return false;

        WriteReg(0x2B, 0x40); // CTRL_REG2: reset
        System::Delay(10);
        WriteReg(0x0E, 0x00); // XYZ_DATA_CFG: +-2g
        WriteReg(0x2B, 0x02); // CTRL_REG2: high resolution
        WriteReg(0x2A, 0x05); // CTRL_REG1: active, low noise
        return true;
    }

    // Reads the sensor and updates x, y, z
    void Read()
    {
        uint8_t buf[6];
        i2c_.ReadDataAtAddress(kAddress << 1, 0x01, 1, buf, 6, 100);

        // 14-bit values, left aligned in two bytes
        int16_t rx = (int16_t)((buf[0] << 8) | buf[1]) >> 2;
        int16_t ry = (int16_t)((buf[2] << 8) | buf[3]) >> 2;
        int16_t rz = (int16_t)((buf[4] << 8) | buf[5]) >> 2;

        // 4096 counts per g at +-2g
        x = rx / 4096.f * 9.80665f;
        y = ry / 4096.f * 9.80665f;
        z = rz / 4096.f * 9.80665f;
    }

  private:
    static const uint8_t kAddress = 0x1D; // A pin floating = 0x1D

    I2CHandle i2c_;

    uint8_t ReadReg(uint8_t reg)
    {
        uint8_t value = 0;
        i2c_.ReadDataAtAddress(kAddress << 1, reg, 1, &value, 1, 100);
        return value;
    }

    void WriteReg(uint8_t reg, uint8_t value)
    {
        i2c_.WriteDataAtAddress(kAddress << 1, reg, 1, &value, 1, 100);
    }
};
