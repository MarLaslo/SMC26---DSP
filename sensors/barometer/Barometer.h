#pragma once

#include "daisy_seed.h"

using namespace daisy;

// Adafruit BMP585 pressure / temperature sensor on I2C1 (SCL = D11, SDA = D12).
// Register map from the Bosch BMP5 Sensor API.
class Barometer
{
  public:
    // Filled in by Read()
    float temperature; // degrees C
    float pressure;    // Pa (divide by 100 for hPa)

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

        WriteReg(0x7E, 0xB6); // CMD: soft reset
        System::Delay(5);

        uint8_t id = ReadReg(0x01); // CHIP_ID: 0x51 = BMP585, 0x50 = BMP581
        if(id != 0x51 && id != 0x50)
            return false;

        uint8_t status = ReadReg(0x28); // STATUS: NVM ready, no NVM error
        if(!(status & 0x02) || (status & 0x04))
            return false;

        WriteReg(0x36, 0x58); // OSR_CONFIG: pressure on, pressure 8x, temp 1x
        WriteReg(0x37, 0xBD); // ODR_CONFIG: 50 Hz, normal mode
        return true;
    }

    // Reads the sensor and updates temperature and pressure
    void Read()
    {
        uint8_t buf[6];
        i2c_.ReadDataAtAddress(kAddress << 1, 0x1D, 1, buf, 6, 100);

        // 24-bit values, low byte first
        int32_t  raw_t = (int32_t)((buf[2] << 24) | (buf[1] << 16) | (buf[0] << 8)) >> 8;
        uint32_t raw_p = (buf[5] << 16) | (buf[4] << 8) | buf[3];

        temperature = raw_t / 65536.f;
        pressure    = raw_p / 64.f;
    }

  private:
    static const uint8_t kAddress = 0x47; // 0x46 if SDO is tied to GND

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
