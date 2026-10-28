#pragma once

#include "daisy_seed.h"

using namespace daisy;

// Adafruit MPR121 12-key capacitive touch sensor on I2C1 (SCL = D11, SDA = D12).
// Based on the Arduino Adafruit_MPR121 library.
//
// Call Read() once every loop, then use the functions below.
// Pads are numbered 0 to 11.
class CapacitiveTouch
{
  public:
    static const int kNumPads = 12;

    // Returns false if the sensor is not found.
    // touch / release: how much a pad must change to count as touched /
    // released. Lower = more sensitive. Touch must be higher than release.
    bool Init(uint8_t touch = 12, uint8_t release = 6)
    {
        I2CHandle::Config cfg;
        cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
        cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        cfg.mode           = I2CHandle::Config::Mode::I2C_MASTER;
        cfg.pin_config.scl = seed::D11;
        cfg.pin_config.sda = seed::D12;
        i2c_.Init(cfg);

        WriteReg(0x80, 0x63); // soft reset
        System::Delay(1);
        WriteReg(0x5E, 0x00); // ECR: stop, so the settings below can be written

        if(ReadReg(0x5D) != 0x24) // CONFIG2 is 0x24 after a reset
            return false;

        // Touch and release thresholds for every pad
        for(uint8_t i = 0; i < kNumPads; i++)
        {
            WriteReg(0x41 + 2 * i, touch);
            WriteReg(0x42 + 2 * i, release);
        }

        // Baseline filtering (same values as the Adafruit library)
        WriteReg(0x2B, 0x01); // MHDR
        WriteReg(0x2C, 0x01); // NHDR
        WriteReg(0x2D, 0x0E); // NCLR
        WriteReg(0x2E, 0x00); // FDLR
        WriteReg(0x2F, 0x01); // MHDF
        WriteReg(0x30, 0x05); // NHDF
        WriteReg(0x31, 0x01); // NCLF
        WriteReg(0x32, 0x00); // FDLF
        WriteReg(0x33, 0x00); // NHDT
        WriteReg(0x34, 0x00); // NCLT
        WriteReg(0x35, 0x00); // FDLT

        WriteReg(0x5B, 0x00); // DEBOUNCE: none
        WriteReg(0x5C, 0x10); // CONFIG1: 16 uA charge current
        WriteReg(0x5D, 0x20); // CONFIG2: 0.5 us charge time, 1 ms sample period

        WriteReg(0x5E, 0x80 + kNumPads); // ECR: baseline tracking, all 12 pads on
        return true;
    }

    // Reads which pads are touched. Call once every loop.
    void Read()
    {
        uint8_t buf[2] = {0, 0};
        i2c_.ReadDataAtAddress(kAddress << 1, 0x00, 1, buf, 2, 10);

        last_    = current_;
        current_ = ((buf[1] << 8) | buf[0]) & 0x0FFF;
    }

    // True while pad is touched
    bool IsTouched(int pad) const { return current_ & (1 << pad); }

    // True only on the loop where pad was touched
    bool JustTouched(int pad) const
    {
        return (current_ & (1 << pad)) && !(last_ & (1 << pad));
    }

    // True only on the loop where pad was let go
    bool JustReleased(int pad) const
    {
        return !(current_ & (1 << pad)) && (last_ & (1 << pad));
    }

    // All 12 pads as bits: bit 0 = pad 0 ... bit 11 = pad 11
    uint16_t Touched() const { return current_; }

    // Raw reading of a pad (0 to 1023), lower when touched.
    // Useful for finding good thresholds or for "how close is the hand".
    uint16_t Raw(int pad)
    {
        uint8_t buf[2] = {0, 0};
        i2c_.ReadDataAtAddress(kAddress << 1, 0x04 + 2 * pad, 1, buf, 2, 10);
        return ((buf[1] << 8) | buf[0]) & 0x03FF;
    }

  private:
    static const uint8_t kAddress = 0x5A; // ADDR pin unconnected = 0x5A

    I2CHandle i2c_;
    uint16_t  current_ = 0;
    uint16_t  last_    = 0;

    uint8_t ReadReg(uint8_t reg)
    {
        uint8_t value = 0;
        i2c_.ReadDataAtAddress(kAddress << 1, reg, 1, &value, 1, 10);
        return value;
    }

    void WriteReg(uint8_t reg, uint8_t value)
    {
        i2c_.WriteDataAtAddress(kAddress << 1, reg, 1, &value, 1, 10);
    }
};
